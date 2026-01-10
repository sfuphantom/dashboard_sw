#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>

#include "vehicleSpeed.h"

// GPIO ports for the 7seg pins
const int segment4[4] = {26, 20, 21, 16};

// Digits
const int digit1 = 5;
const int digit2 = 6;

// Number patterns for 7-segment display (assuming a common anode configuration)
const int numbers[10][4] = {
    {0, 0, 0, 0}, // 0
    {0, 0, 0, 1}, // 1
    {0, 0, 1, 0}, // 2
    {0, 0, 1, 1}, // 3
    {0, 1, 0, 0}, // 4
    {0, 1, 0, 1}, // 5
    {0, 1, 1, 0}, // 6
    {0, 1, 1, 1}, // 7
    {1, 0, 0, 0}, // 8
    {1, 0, 0, 1}  // 9
};

void print_segment(int num)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        digitalWrite(segment4[i], numbers[num][i]);
    }
}

void printSpeed(int speed_sensor_data)
{
    // Note: Our dispaly onyl has two two digits, therefore we can only dispay nums 0-99
    if (speed_sensor_data < 0)
        speed_sensor_data = 0;
    else if (speed_sensor_data > 99)
        speed_sensor_data = 99;

    int i;                    // Loop variable
    for (i = 0; i < 100; i++) // Loop for persistence of vision effect
    {
        int first_digit = speed_sensor_data / 10;
        digitalWrite(digit1, HIGH); // Turn on Digit One
        print_segment(first_digit); // Print number on segment 1
        usleep(100);
        digitalWrite(digit1, LOW); // Turn off Digit One

        int second_digit = speed_sensor_data % 10;
        digitalWrite(digit2, HIGH);  // Turn on Digit Two
        print_segment(second_digit); // Print number on segment 2
        usleep(100);
        digitalWrite(digit2, LOW); // Turn off Digit Two
    }
    // Intentionally no per-refresh logging to avoid console spam.
}

void bootup()
{
    int test;
    for (test = 0; test < 100; test++)
    {
        int i;
        for (i = 0; i < 100; i++) // Loop for persistence of vision effect
        {
            int first_digit = test / 10;
            // Display number on first digit
            digitalWrite(digit1, HIGH); // Turn on Digit One
            digitalWrite(digit2, LOW);  // Ensure Digit Two is off
            print_segment(first_digit); // Print number on segment
            usleep(100);                // Short delay

            int second_digit = test % 10;
            // Display number on second digit
            digitalWrite(digit1, LOW);   // Ensure Digit One is off
            digitalWrite(digit2, HIGH);  // Turn on Digit Two
            print_segment(second_digit); // Print number on segment
            usleep(100);                 // Short delay
        }
    }

    digitalWrite(digit1, LOW); // Turn off Digit One
    digitalWrite(digit2, LOW); // Turn off Digit Two
}

int init7Seg(void)
{
    if (wiringPiSetupGpio() == -1)
    {
        fprintf(stderr, "Failed to initialize WiringPi GPIO\n");
        return -1;
    }
    int i;

    for (i = 0; i < 4; i++)
    {
        pinMode(segment4[i], OUTPUT);
        digitalWrite(segment4[i], LOW);
    }

    pinMode(digit1, OUTPUT);
    digitalWrite(digit1, LOW);

    pinMode(digit2, OUTPUT);
    digitalWrite(digit2, LOW);

    bootup();
    return 0;
}

// int main(void)
// {
//     init7Seg();

//     // printSpeed(42); // Example usage with a speed of 42

//     return 0;
// }
