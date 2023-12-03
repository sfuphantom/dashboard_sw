import seg7
import dac

speed_sensor_data = 36 # Any number from 0 to 99 for speed
soc_decimal_sensor_data = 255 # Range from 0 to 255. 255 is fully lit, 0 is fully off

dac.init_spi()

try:
	dac.state_of_charge(soc_decimal_sensor_data)
	seg7.test()
	seg7.speed(speed_sensor_data)
finally:
    dac.close_spi()
    print("Closed SPI")