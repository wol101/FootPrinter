#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import RPi.GPIO as GPIO
import time

def sensor_test():
    sensor_pin = 23;
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(sensor_pin, GPIO.IN)

    for i in range(0, 1000):
        print('Pin %d = %d' % (sensor_pin, GPIO.input(sensor_pin)))
        time.sleep(0.01)

# program starts here
if __name__ == "__main__":
    sensor_test()
