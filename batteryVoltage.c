#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include "batteryVoltage.h"

// minimum battery voltage = 297.6 V, CAN = 0 (0% full)
// maximum battery voltage = 403.2 V, CAN = 65535 (100% full)
// anything inbetween = linear interpolation

// Must treat CAN value as 01 fraction and scale it to bar indicator 

const int CS = 24;
const int SPI_CHANNEL = 1;
const int SPI_SPEED = 20000000;

void state_of_charge(uint8_t soc_decimal_sensor_data)
{
    printf("Battery Charge value: %d\n", soc_decimal_sensor_data);

    uint8_t binary_value = soc_decimal_sensor_data; // byte is bit pattern 
    uint8_t data[] = {0x00, 0x00, binary_value};

    digitalWrite(CS, LOW); // selecting device
    int rc = wiringPiSPIDataRW(SPI_CHANNEL, data, sizeof(data));
    digitalWrite(CS, HIGH); // deselecting device

    // Ensure that SPI_CHANNEL and data are correct
    if (rc == -1)
    {
        fprintf(stderr, "Error: SPI data transfer failed\n");
        return;
    }
}

void printBatteryVoltage(int soc_decimal_sensor_data)
{
    state_of_charge((uint8_t)soc_decimal_sensor_data);
    return;
}
void batteryVoltageBootup()
{
    int test;
    for (test = 0; test < 255; test++)
    {
        state_of_charge(test);
        usleep(10000);
    }
    usleep(1000000); // Stays fully lit for 1 seconds
    state_of_charge(0);
}

int initSPI(void)
{
    if (wiringPiSetupGpio() == -1)
    {
        fprintf(stderr, "Failed to initialize WiringPi GPIO\n");
        return -1;
    }
    if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        fprintf(stderr, "Failed to initialize WiringPi SPI\n");
        return -1;
    }
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH); // deselecting device

    batteryVoltageBootup();
    return 0;
}
