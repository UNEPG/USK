import time
from usk.Sensor import UniSense

sensor = UniSense()
sensor.print_attrs()
while True:
    sensor.update_data()
    print(sensor.temperature_bme)
    print(sensor.temperature_bno)
    print(sensor.humidity)
    print(sensor.pressure)
    print(sensor.acceleration)
    print(sensor.gyro)
    print(sensor.magnetic)

    time.sleep(10)
