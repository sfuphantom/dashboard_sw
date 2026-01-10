#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>

// decode module and a unit test harness that covers speed decoding (including negative), 
// battery decoding (0x0000/0xFFFF/midpoints), and clamp/rounding rules

float decode_speed_kmh(uint8_t lo, uint8_t hi);
uint8_t decode_battery_bar(uint8_t lo, uint8_t hi);
int clamp_speed_display(float kmh);

#endif
