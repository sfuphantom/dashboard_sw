#ifndef batteryVoltage_h
#define batteryVoltage_h

extern void batteryVoltageBootup(void);
extern int initSPI(void);
extern void printBatteryVoltage(int soc_decimal_sensor_data);

#endif
