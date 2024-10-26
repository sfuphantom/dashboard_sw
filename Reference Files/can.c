
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
#include <stdio.h>
#include <unistd.h>

#include "vehicleSpeed.h"

void initCAN()
{
    int s; // Socket descriptor
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

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

    // // Prepare a CAN frame
    // frame.can_id = 0x123; // CAN message ID
    // frame.can_dlc = 2;    // Data length code
    // frame.data[0] = 0xAA; // Data byte 0
    // frame.data[1] = 0xBC; // Data byte 1

    // // Send the CAN frame
    // if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
    // {
    //     perror("Write error");
    //     return 1;
    // }

    while (1)
    {
        init7Seg();
        // Receive a CAN frame
        if (read(s, &frame, sizeof(struct can_frame)) < 0)
        {
            perror("Read error");
            printf(frame.data);
            return 1;
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
            printSpeed(valRecieved);
        }
        printf("\n");
    }
    // Close the CAN socket
    close(s);

    return 0;
}
