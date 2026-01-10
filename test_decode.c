#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "decode.h"

static void test_speed_decode(void)
{
    float s0 = decode_speed_kmh(0x00, 0x00);
    assert(fabsf(s0 - 0.0f) < 0.0001f);

    float s1 = decode_speed_kmh(0x64, 0x00); // 100
    assert(fabsf(s1 - 3.6f) < 0.0001f);

    float s2 = decode_speed_kmh(0xE8, 0x03); // 1000
    assert(fabsf(s2 - 36.0f) < 0.0001f);

    float s3 = decode_speed_kmh(0x9C, 0xFF); // -100
    assert(fabsf(s3 + 3.6f) < 0.0001f);
}

static void test_battery_decode(void)
{
    uint8_t b0 = decode_battery_bar(0x00, 0x00);
    assert(b0 == 0);

    uint8_t b1 = decode_battery_bar(0xFF, 0xFF);
    assert(b1 == 255);

    uint8_t b2 = decode_battery_bar(0x00, 0x80); // 0x8000
    assert(b2 == 127);

    uint8_t b3 = decode_battery_bar(0xFF, 0x7F); // 0x7FFF
    assert(b3 == 127);
}

static void test_speed_clamp_round(void)
{
    assert(clamp_speed_display(9.6f) == 10);
    assert(clamp_speed_display(9.4f) == 9);
    assert(clamp_speed_display(-1.0f) == 0);
    assert(clamp_speed_display(123.4f) == 99);
}

int main(void)
{
    test_speed_decode();
    test_battery_decode();
    test_speed_clamp_round();
    printf("decode tests passed\n");
    return 0;
}
