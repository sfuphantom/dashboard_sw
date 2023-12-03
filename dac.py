
# MCP48FVB0X 8-Bit DAC

#CS2 -> GPIO07 Physical Pin 26 (SPI0 CE1) => Can be changed
#SDI -> GPIO10 Physical Pin 19 (SPI0 MOSI) => cannot be changed in hardware SPI MODE
#SCK -> GPIO11 Physical Pin 23 (SPI0 SCLK) => cannot be changed in hardware SPI MODE

from time import sleep
import RPi.GPIO as GPIO
import spidev

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

cs = 26 # Defined by pin out

spi = spidev.SpiDev()

def init_spi():
    spi.open(0, 1)
    spi.max_speed_hz = 20000000
    GPIO.setup(cs, GPIO.OUT)

def close_spi():
    spi.close()

def convert_to_binary(soc_decimal_sensor_data):
    # Ensure the value is within the valid range
    if 0 <= soc_decimal_sensor_data <= 255:
        # Convert the decimal value to binary with leading zeros
        binary_representation = format(soc_decimal_sensor_data, '08b')

        return binary_representation
    else:
        raise ValueError("Value must be between 0 and 255")
    
def state_of_charge(soc_decimal_sensor_data):
    binary_result = convert_to_binary(soc_decimal_sensor_data)
    GPIO.output(cs,0)
    print("Changed light bar brightness")
    data = [0b00000000, 0b00000000, int(binary_result, 2)]
    spi.writebytes(data)
    sleep(1)
    GPIO.output(cs,0)
