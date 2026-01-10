#include <stdio.h>
#include <unistd.h>
#include "batteryVoltage.h"

int initSPI(void);

int main(void)
{
    if (initSPI() != 0)
    {
        fprintf(stderr, "initSPI failed\n");
        return 1;
    }
    return 0;
}
