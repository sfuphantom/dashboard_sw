
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
#include <pthread.h>

#include "vehicleSpeed.h"
#include "batteryVoltage.h"

void init7Seg(void);
void printSpeed(int speed);
void printBatteryVoltage(int voltage);

void batteryVoltageBootup(void);
void initSPI(void);

int s; // Socket descriptor
struct sockaddr_can addr;
struct ifreq ifr;
struct can_frame frame;

// These act as semaphores to control the flow of the program
bool isProcessingSpeedFrame = false;   // Flag to indicate if the program is processing speed frames
bool isProcessingBatteryFrame = false; // Flag to indicate if the program is processing battery frames

pthread_mutex_t speedMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t batteryMutex = PTHREAD_MUTEX_INITIALIZER;

// Wrapper functions to satisfy pthread_create signature
void *init7SegWrapper(void *arg)
{
    init7Seg();
    return NULL;
}

void *initSPIWrapper(void *arg)
{
    initSPI();
    return NULL;
}

int initCAN()
{

    // Open a socket for CAN communication
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("Socket creation error");
        return 1;
    }

    // Specify the CAN interface name (vcan0 for virtual CAN interface)
    strcpy(ifr.ifr_name, "can0");

    // Get the interface index
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("ioctl error");
        return 1;
    }

    // Bind the socket to the CAN interface
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind error");
        return 1;
    }
    return 0;
}

void *initCANWrapper(void *arg)
{
    initCAN();
    return NULL;
}

void receiveCANSpeedFrame()
{
    while (1) // loops until finds frame we're looking for
    {
        printf("Receiving Speed Frame\n");
        // Receive a CAN frame
        // if (read(s, &frame, sizeof(struct can_frame)) < 0)
        // {
        //     printf("Error reading CAN frame\n");
        //     perror("Read error");
        //     printf("data: %d\n", frame.data[0]);

        //     return;
        // }
        // Display received CAN frame data
        printf("Received CAN frame:\n");
        printf("ID: 0x%X\n", frame.can_id);
        printf("DLC: %d\n", frame.can_dlc);

        if (frame.can_id == 2) // id for SPEED
        {
            printf("Data: ");

            for (int i = 0; i < frame.can_dlc; i++)
            {
                printf("0x%02X\n", frame.data[i]);
                int valReceived = frame.data[i];
                printf("Integer Value: %d\n", valReceived);
                printf("\n");

                // Display the speed on the 7-segment display
                printSpeed(valReceived);
                return;
            }
        }
        usleep(500000); // Delay to reduce CPU usage (0.5seconds)
    }
}

void receiveCANBatteryFrame()
{
    while (1) // loops until finds frame we're looking for
    {
        printf("Receiving Battery Frame\n");

        // // Receive a CAN frame
        // if (read(s, &frame, sizeof(struct can_frame)) < 0)
        // {
        //     perror("Read error");
        //     printf("data: %d\n", frame.data[0]);
        //     return;
        // }
        // Display received CAN frame data
        printf("Received CAN frame:\n");
        printf("ID: 0x%X\n", frame.can_id);
        printf("DLC: %d\n", frame.can_dlc);

        if (frame.can_id == 6) // Arbitrary id for BATTERY
        {
            printf("Data: ");

            for (int i = 0; i < frame.can_dlc; i++)
            {
                printf("0x%02X\n", frame.data[i]);
                int valReceived = frame.data[i];
                printf("Integer Value: %d\n", valReceived);
                printf("\n");

                // Display the speed on the 7-segment display
                printBatteryVoltage(valReceived);
                return;
            }
        }
        usleep(500000); // Delay to reduce CPU usage (0.5seconds)
    }
}
void *speedFrameThreadFunction(void *arg)
{
    while (1)
    {
        printf("1\n");
        pthread_mutex_lock(&speedMutex);
        if (!isProcessingSpeedFrame)
        {
            isProcessingSpeedFrame = true;
            pthread_mutex_unlock(&speedMutex);
            receiveCANSpeedFrame(); // Process speed frames in this function
            pthread_mutex_lock(&speedMutex);
            isProcessingSpeedFrame = false; // Reset flag after processing
        }
        pthread_mutex_unlock(&speedMutex);
        usleep(500000);
    }
}

void *batteryFrameThreadFunction(void *arg)
{
    while (1)
    {
        printf("2\n");

        pthread_mutex_lock(&batteryMutex);
        if (!isProcessingBatteryFrame)
        {
            isProcessingBatteryFrame = true;
            pthread_mutex_unlock(&batteryMutex);
            receiveCANBatteryFrame(); // Process battery frames in this function
            pthread_mutex_lock(&batteryMutex);
            isProcessingBatteryFrame = false; // Reset flag after processing
        }
        pthread_mutex_unlock(&batteryMutex);
        usleep(500000);
    }
}
int main()
{
    pthread_mutex_init(&speedMutex, NULL);
    pthread_mutex_init(&batteryMutex, NULL);
    pthread_t displayBatteryThread, displaySpeedThread;
    // pthread_t init7SegThread, initSPIThread, initCANThread;
    pthread_t init7SegThread, initSPIThread;

    // pthread_create(&initCANThread, NULL, initCANWrapper, NULL);
    pthread_create(&init7SegThread, NULL, init7SegWrapper, NULL);
    pthread_create(&initSPIThread, NULL, initSPIWrapper, NULL);

    // init7Seg();
    // initSPI();
    // initCAN();

    // pthread_join(initCANThread, NULL);
    pthread_join(init7SegThread, NULL);
    pthread_join(initSPIThread, NULL);
    printf("2 init threads joined backed to main\n");
    // Initialize the two main threads
    printf("Start of program\n");

    // if (initCAN != 0){
    //     fprinf(stderr, "Failed to initialize CAN interface\n");
    //     exit(1);
    // }

    if (pthread_create(&displaySpeedThread, NULL, speedFrameThreadFunction, NULL) != 0)
    {
        perror("Failed to create speed frame thread\n");
        exit(1);
    }
    if (pthread_create(&displayBatteryThread, NULL, batteryFrameThreadFunction, NULL) != 0)
    {
        perror("Failed to create battery frame thread\n");
        exit(1);
    }

    // While loop needed otherwise program terminates through main thread.
    while (1)
    {
    }

    // Close the CAN socket
    // close(s);
}