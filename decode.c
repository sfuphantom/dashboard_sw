#include "decode.h"

// decode module and a unit test harness that covers speed decoding (including negative), 
// battery decoding (0x0000/0xFFFF/midpoints), and clamp/rounding rules

float decode_speed_kmh(uint8_t lo, uint8_t hi)
{
    int16_t raw = (int16_t)((hi << 8) | lo);
    return (raw / 100.0f) * 3.6f;
}

uint8_t decode_battery_bar(uint8_t lo, uint8_t hi)
{
    uint16_t raw = (uint16_t)((hi << 8) | lo);
    double fraction = raw / 65535.0;
    double scaled = fraction * 255.0;
    int bar = (int)scaled;

    if (bar < 0) return 0;
    if (bar > 255) return 255;
    return (uint8_t)bar;
}

int clamp_speed_display(float kmh)
{
    int rounded;

    if (kmh >= 0.0f)
    {
        rounded = (int)(kmh + 0.5f);
    }
    else
    {
        rounded = (int)(kmh - 0.5f);
    }

    if (rounded < 0) return 0;
    if (rounded > 99) return 99;
    return rounded;
}
