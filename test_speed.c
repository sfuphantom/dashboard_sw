#include <stdio.h>
#include <unistd.h>
#include "vehicleSpeed.h"

int init7Seg(void);

int main(void)
{
    if (init7Seg() != 0)
    {
        fprintf(stderr, "init7Seg failed\n");
        return 1;
    }

    for (int x = 0; x <= 99; x++)
    {
        printSpeed(x);
        usleep(200000); // 0.2s
    }
    return 0;
}
