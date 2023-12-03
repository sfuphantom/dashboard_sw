import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

#GPIO ports for the 7seg pins
segment4 = (26,20,21,16)

for segment in segment4:
    GPIO.setup(segment, GPIO.OUT)
    GPIO.output(segment, 0)

#Digit 1
GPIO.setup(5, GPIO.OUT)
GPIO.output(5, 0) #Off initially
#Digit 2
GPIO.setup(6, GPIO.OUT)
GPIO.output(6, 0) #Off initially

null = [0,0,0,0]
zero = [0,0,0,0]
one = [0,0,0,1]
two = [0,0,1,0]
three = [0,0,1,1]
four = [0,1,0,0]
five = [0,1,0,1]
six = [0,1,1,0]
seven = [0,1,1,1]
eight = [1,0,0,0]
nine = [1,0,0,1]


def print_segment(num):
    if num == 1:
        for i in range(4):
            GPIO.output(segment4[i], one[i])

    if num == 2:
        for i in range(4):
            GPIO.output(segment4[i], two[i])

    if num == 3:
        for i in range(4):
            GPIO.output(segment4[i], three[i])

    if num == 4:
        for i in range(4):
            GPIO.output(segment4[i], four[i])

    if num == 5:
        for i in range(4):
            GPIO.output(segment4[i], five[i])

    if num == 6:
        for i in range(4):
            GPIO.output(segment4[i], six[i])

    if num == 7:
        for i in range(4):
            GPIO.output(segment4[i], seven[i])

    if num == 8:
        for i in range(4):
            GPIO.output(segment4[i], eight[i])

    if num == 9:
        for i in range(4):
            GPIO.output(segment4[i], nine[i])

    if num == 0:
        for i in range(4):
            GPIO.output(segment4[i], zero[i])        
            
    return


delay = 0.001 #0.001 second delay

#test all digits
def test():
    for test in range (0,10):
        GPIO.output(5, 1) #Turn on Digit One
        print_segment (test) #Print number on segment 1
        GPIO.output(6, 1) #Turn on Digit Two
        print_segment (test) #Print number on segment 2
        time.sleep(0.15)
    GPIO.output(5, 0) #Turn off Digit One
    GPIO.output(6, 0) #Turn off Digit Two
        
    time.sleep(1)

def speed(speed_sensor_data):
    for x in range(1000):
        first_digit = speed_sensor_data // 10
        GPIO.output(5, 1) #Turn on Digit One
        print_segment (first_digit) #Print number on segment 1
        time.sleep(delay)
        GPIO.output(5, 0) #Turn off Digit One
        
        second_digit = speed_sensor_data % 10
        GPIO.output(6, 1) #Turn on Digit Two
        print_segment (second_digit) #Print number on segment 2
        time.sleep(delay)
        GPIO.output(6, 0) #Turn off Digit Two
    print("Speed Displayed")
    

