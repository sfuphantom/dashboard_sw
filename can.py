# so allegendly the linux kernel suppoers CAn and inscludes socketCan drivers for the MCP2515 that we are using

# https://www.hackster.io/youness/how-to-connect-raspberry-pi-to-can-bus-b60235
# https://www.beyondlogic.org/adding-can-controller-area-network-to-the-raspberry-pi/
from time import sleep
import RPi.GPIO as GPIO
import spidev

spi = spidev.SpiDev()

def init_spi():
    spi.open(0, 2)  # Open another connection
    # Connects to the specified SPI device, opening /dev/spidev<bus>.<device>
    spi.max_speed_hz = 10000000  # Set the speed to 10MHz
