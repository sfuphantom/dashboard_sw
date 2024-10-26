#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include "batteryVoltage.h"

const int CS = 24;
const int SPI_CHANNEL = 1;
const int SPI_SPEED = 20000000;

char *convert_to_binary(uint8_t soc_decimal_sensor_data)
{
    static char binary_representation[9];
    if (soc_decimal_sensor_data >= 0 && soc_decimal_sensor_data <= 255)
    {
        for (int i = 7; i >= 0; i--)
        {
            binary_representation[i] = (soc_decimal_sensor_data % 2) + '0';
            soc_decimal_sensor_data /= 2;
        }
        return binary_representation;
    }
    else
    {
        printf("Value must be between 0 and 255\n");
        return NULL;
    }
}

void state_of_charge(uint8_t soc_decimal_sensor_data)
{
    char *binary_result = convert_to_binary(soc_decimal_sensor_data);
    if (binary_result == NULL)
    {
        fprintf(stderr, "Error: convert_to_binary returned NULL\n");
        return;
    }

    printf("Binary representation: %s\n", binary_result);

    // Ensure that the binary representation fits in a uint8_t
    // For simplicity, assume binary_result is at most 8 bits
    uint8_t binary_value = (uint8_t)strtol(binary_result, NULL, 2);

    // Clean up binary_result if it was dynamically allocated
    // free(binary_result);

    uint8_t data[] = {0x00, 0x00, binary_value};

    // Ensure that SPI_CHANNEL and data are correct
    if (wiringPiSPIDataRW(SPI_CHANNEL, data, sizeof(data)) == -1)
    {
        fprintf(stderr, "Error: SPI data transfer failed\n");
        return;
    }

    printf("Changed light bar brightness\n");
    digitalWrite(CS, HIGH);
}

void batteryVoltageBootup()
{
    int test;
    int i;
    for (test = 0; test < 255; test++)
    {
        for (i = 0; i < 100; i++) // Loop for persistence of vision effect
        {
            state_of_charge(test);
            usleep(10);
        }
    }
    state_of_charge(0);
}

void initSPI()
{

    wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED);
    pinMode(CS, OUTPUT);

    batteryVoltageBootup();
}

// int main()
// {

// if (wiringPiSetup() == -1)
// {
//     printf("Error initializing WiringPi\n");
//     return;
// }
//     initSPI();

//     uint8_t sensor_data = 255; // Example sensor data
//     state_of_charge(sensor_data);

//     return 0;
// }
