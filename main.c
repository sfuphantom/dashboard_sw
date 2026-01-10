
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <wiringPi.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <errno.h> 

#include "vehicleSpeed.h"
#include "batteryVoltage.h"
#include "decode.h"

int init7Seg(void);
int initSPI(void);

void printBatteryVoltage(int voltage);
void batteryVoltageBootup(void);

static long ms_since(const struct timespec *now, const struct timespec *prev)
{
    return (now->tv_sec - prev->tv_sec) * 1000L +
           (now->tv_nsec - prev->tv_nsec) / 1000000L;
}

// Shared State
typedef struct
{
    float speed_kmh;        // latest decoded speed
    uint8_t battery_bar;    // 0..255 

    bool has_speed;
    bool has_battery;
    
    struct timespec last_speed_ts;
    struct timespec last_battery_ts;

    pthread_mutex_t m;
    pthread_cond_t  cv;     // signal when new data arrives
} SharedState;

static SharedState g_state = {
    .speed_kmh = 0.0f,
    .battery_bar = 0,
    .has_speed = false,
    .has_battery = false,
    .m = PTHREAD_MUTEX_INITIALIZER,
    .cv = PTHREAD_COND_INITIALIZER
};

// CAN Globals 

static int CAN_socket = -1;
static struct sockaddr_can g_addr;
static struct ifreq g_ifr;

static int initCAN(const char *ifname)
{
    CAN_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (CAN_socket < 0)
    {
        perror("CAN socket()");
        return -1;
    }

    // bind to interface (e.g., "can0")
    memset(&g_ifr, 0, sizeof(g_ifr));
    strncpy(g_ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(CAN_socket, SIOCGIFINDEX, &g_ifr) < 0)
    {
        perror("CAN ioctl(SIOCGIFINDEX)");
        close(CAN_socket);
        CAN_socket = -1;
        return -1;
    }

    memset(&g_addr, 0, sizeof(g_addr));
    g_addr.can_family  = AF_CAN;
    g_addr.can_ifindex = g_ifr.ifr_ifindex;

    if (bind(CAN_socket, (struct sockaddr *)&g_addr, sizeof(g_addr)) < 0)
    {
        perror("CAN bind()");
        close(CAN_socket);
        CAN_socket = -1;
        return -1;
    }

    return 0;
}

static void handleSpeedFrame(const struct can_frame *f)
{
    // ID == 0x02, speed in data[0..1] little-endian, scaled.
    float speed_kmh = decode_speed_kmh(f->data[0], f->data[1]);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    pthread_mutex_lock(&g_state.m);
    g_state.speed_kmh = speed_kmh;
    g_state.has_speed = true;
    g_state.last_speed_ts = now;
    pthread_cond_broadcast(&g_state.cv);
    pthread_mutex_unlock(&g_state.m);
}

static void handleBatteryFrame(const struct can_frame *f)
{
    // ID == 0x80, raw 0..65535 maps 297.6..403.2
    uint8_t barLevel = decode_battery_bar(f->data[0], f->data[1]);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    pthread_mutex_lock(&g_state.m);
    g_state.battery_bar = barLevel;
    g_state.has_battery = true;
    g_state.last_battery_ts = now;
    pthread_cond_broadcast(&g_state.cv);
    pthread_mutex_unlock(&g_state.m);
}

static void *canReaderThread(void *arg)
{
    (void)arg;
    struct can_frame frame;

    while (1)
    {
        ssize_t n = read(CAN_socket, &frame, sizeof(frame));
        if (n < 0)
        {
            if (errno == EINTR) continue;  // interrupted, try again
            perror("CAN read()");
            usleep(100 * 1000);           // avoid tight error loop
            continue;
        }
        if (n != (ssize_t)sizeof(frame))
        {
            fprintf(stderr, "CAN: short read (%zd)\n", n);
            continue;
        }

        if (frame.can_dlc < 2)
        {
            fprintf(stderr, "CAN: short frame DLC=%u\n", frame.can_dlc);
            continue;
        }

        uint32_t can_id = frame.can_id & CAN_SFF_MASK;
        if (can_id == 0x02)
        {
            handleSpeedFrame(&frame);
        }
        else if (can_id == 0x80)
        {
            handleBatteryFrame(&frame);
        }
        // else ignore other CAN IDs
    }

    return NULL;
}


// Consumer threads for physical deisplay 

// speed display thread -> wakes when speed updates
static void *speedDisplayThread(void *arg)
{
    (void)arg;

    int last_printed = -1;

    while (1)
    {
        pthread_mutex_lock(&g_state.m);
        while (!g_state.has_speed)
        {
            pthread_cond_wait(&g_state.cv, &g_state.m);
        }

        float s = g_state.speed_kmh;
        pthread_mutex_unlock(&g_state.m);

        // print/update only if changed meaningfully -> simple despam
        int display_speed = clamp_speed_display(s);
        if (display_speed != last_printed)
        {
            // If your display function name is different, change this:
            printSpeed(display_speed);
            last_printed = display_speed;
        }

        // tiny sleep -> avoids hammering display if frames are rly fast
        usleep(20 * 1000);
    }
    return NULL;
}

// Battery display thread: non-blocking, using rate-limit
static void *batteryDisplayThread(void *arg)
{
    (void)arg;

    uint8_t last_bar = 0xFF;
    struct timespec last_update = {0};

    while (1)
    {
        pthread_mutex_lock(&g_state.m);
        while (!g_state.has_battery)
        {
            pthread_cond_wait(&g_state.cv, &g_state.m);
        }
        uint8_t bar = g_state.battery_bar;
        pthread_mutex_unlock(&g_state.m);

        // Rate limit battery display 
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        if (ms_since(&now, &last_update) >= 1000) // 1 seconds
        {
            if (bar != last_bar)
            {
                printBatteryVoltage((int)bar);
                last_bar = bar;
            }
            last_update = now;
        }

        usleep(20 * 1000);
    }
    return NULL;
}


int main(void)
{
    // init hardware (sequential to avoid double/parallel WiringPi init)
    if (init7Seg() != 0)
    {
        fprintf(stderr, "Failed to initialize 7-seg\n");
        return 1;
    }
    if (initSPI() != 0)
    {
        fprintf(stderr, "Failed to initialize SPI\n");
        return 1;
    }

    if (initCAN("can0") != 0)
    {
        fprintf(stderr, "Failed to initialize CAN interface\n");
        return 1;
    }

    printf("Start of program\n");

    // start threads
    pthread_t t_can, t_speed, t_batt;
    if (pthread_create(&t_can, NULL, canReaderThread, NULL) != 0)
    {
        perror("pthread_create(canReaderThread)");
        return 1;
    }
    if (pthread_create(&t_speed, NULL, speedDisplayThread, NULL) != 0)
    {
        perror("pthread_create(speedDisplayThread)");
        return 1;
    }
    if (pthread_create(&t_batt, NULL, batteryDisplayThread, NULL) != 0)
    {
        perror("pthread_create(batteryDisplayThread)");
        return 1;
    }

    // main thread
    pthread_join(t_can, NULL);
    pthread_join(t_speed, NULL);
    pthread_join(t_batt, NULL);

    // cleanup
    if (CAN_socket >= 0) close(CAN_socket);
    return 0;
}
