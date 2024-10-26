
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
#include <omp.h>

#include "vehicleSpeed.h"
#include "batteryVoltage.h"

void init7Seg(void);
void printSpeed(int speed);

void batteryVoltageBootup(void);
void initSPI(void);

int s; // Socket descriptor
struct sockaddr_can addr;
struct ifreq ifr;
struct can_frame frame;

// These act as semaphores to control the flow of the program
bool isProcessingSpeedFrame = false;   // Flag to indicate if the program is processing speed frames
bool isProcessingBatteryFrame = false; // Flag to indicate if the program is processing battery frames

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

void recieveCANSpeedFrame()
{
    // Receive a CAN frame
    if (read(s, &frame, sizeof(struct can_frame)) < 0)
    {
        perror("Read error");
        printf("data: %d\n", frame.data[0]);

        return;
    }
    // Display received CAN frame data
    printf("Received CAN frame:\n");
    printf("ID: 0x%X\n", frame.can_id);
    printf("DLC: %d\n", frame.can_dlc);
    printf("Data: ");

    for (int i = 0; i < frame.can_dlc; i++)
    {
        printf("0x%02X\n", frame.data[i]);
        int valRecieved = frame.data[i];
        printf("Integer Value: %d\n", valRecieved);

        // Display the speed on the 7-segment display
        printSpeed(valRecieved);
    }
    printf("\n");
}

int main()
{

#pragma omp parallel sections num_threads(2)
    {

// #pragma omp section
//             {
//                 initCAN(); // Run initCAN on one thread
//             }
#pragma omp section
        {
            init7Seg(); // Run init7Seg on another thread
        }

#pragma omp section
        {
            initSPI(); // Run initSPI on another thread
        }
    }

    while (1)
    {
#pragma omp parallel sections
        {

#pragma omp section
            {

                if (!isProcessingBatteryFrame)
                {
                    printf("idSpeed = %d, \n", omp_get_thread_num());

                    isProcessingBatteryFrame = true;
                    // recieveCANBatteryFrame(); // Receive CAN frame and process it

                    printf("Battery Frame\n");
                    isProcessingBatteryFrame = false;
                }
            }
#pragma omp section
            {

                if (!isProcessingSpeedFrame)
                {
                    printf("idFrame = %d, \n", omp_get_thread_num());

                    isProcessingSpeedFrame = true;
                    // recieveCANSpeedFrame(); // Receive CAN frame and process it
                    printf("Speed Frame\n");
                    isProcessingSpeedFrame = false;
                }
            }
        }

        // // Close the CAN socket
        // close(s);
    }
}