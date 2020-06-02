import os
import subprocess
import re
import smbus


# Version 1
# p = subprocess.Popen(['i2cdetect', '-y', '1'], stdout=subprocess.PIPE, )
#
# for i in range(0, 9):
#     line = str(p.stdout.readline())
#
#     for match in re.finditer("[0-9][0-9]:.*[0-9][0-9]", line):
#         print(match.group())

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

