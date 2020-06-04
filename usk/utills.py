import os
import subprocess
import re
import smbus
from gpiozero import CPUTemperature


def i2c_scan():
    # !/usr/bin/env python
    i2c_devices = []
    bus = smbus.SMBus(1)  # 1 indicates /dev/i2c-1
    for device in range(128):
        try:
            bus.read_byte(device)
            i2c_devices.append(hex(device))
        except:  # exception if read_byte fails
            pass
    return i2c_devices


def get_pi_temp():
    temp = os.popen("vcgencmd measure_temp | egrep -o '[0-9]*\.[0-9]*'").readline()
    print(temp.replace("temp=", ''))


def pi_temp():
    with open('/sys/class/thermal/thermal_zone0/temp') as f:
        temp = f.read()
    print(temp)
    return temp


def cpu_temp():
    cpu = CPUTemperature()
    return cpu.temperature


cpu_temp()