from usk.Sensor import UniSense

sensor = UniSense()
# sensor.print_attrs()
# sensor.ok()
sensor.update_data()
print(sensor.temperature_bme)
print(sensor.temperature_bno)
print(sensor.pressure)
print(sensor.UVindex)
print(sensor.humidity)
print(sensor.linear_acceleration)
print(sensor.IR)
print(sensor.magnetic)
sensor.print_attrs()

from usk.utills import get_pi_temp, pi_temp

temp = get_pi_temp()
print(temp)


t2 = pi_temp()
print(t2)