from .utills import i2c_scan


class UniSense(object):
    def __init__(self):
        # We have two sensors that can read temperature, one is bme 680, and another one is bno055. We will try to read
        # temperature from both of them when ideal, if we can, we return the mean value as the returned value, if one of
        # them is not working, then we use the working sensor to read data.
        self.temperature_bme = ''
        self.temperature_bno = ''
        self.pressure = ''
        self.humidity = ''
        self.air_quality = ''
        self.acceleration = ''
        self.magnetic = ''
        self.gyro = ''
        self.euler = ''
        self.quaternion = ''
        self.linear_acceleration = ''
        self.gravity = ''
        self.air_quality = ''
        self.Vis = ''
        self.IR = ''
        self.UVindex = ''
        self.i2c_devices = ''
        self._bme_ok = False
        self._bno_ok = False
        self._si_ok = False
        self.last_update = ""
        self._init_sensors()

    def print_attrs(self):
        """ Return all available sensor attributes """
        for attribute in self.__dict__.keys():
            print(attribute)

    def get_attrs(self):
        """ Return all available sensor attributes """
        attrs = []
        for attribute in self.__dict__.keys():
            attrs.append(attribute)

    def scan(self):
        self.i2c_devices = i2c_scan()

    def _init_sensors(self):
        for each in ['BME680', 'BNO055', 'SI1145']:
            self.dev_ok(each)

    def dev_ok(self, device):
        self.scan()
        if device == 'BME680':
            if '0x77' in self.i2c_devices:
                self._bme_ok = True
                return True
            else:
                return False
        elif device == 'BNO055':
            if '0x28' in self.i2c_devices:
                self._bno_ok = True
                return True
            else:
                return False
        elif device == 'SI1145':
            if '0x60' in self.i2c_devices:
                self._bno_ok = True
                return True
            else:
                return False

    def bme_ok(self):
        if '0x77' in self.i2c_devices:
            self._bme_ok = True

    def bno_ok(self):
        if '0x28' in self.i2c_devices:
            self._bno_ok = True

    def si_ok(self):
        if '0x60' in self.i2c_devices:
            self._bno_ok = True

    def ok(self):
        self.scan()
        status = {}
        print(f"Found {len(self.i2c_devices)} on the i2c bus")
        for each in ['BME680', 'BNO055', 'SI1145']:
            status[each] = self.dev_ok(each)
        print(status)

    def update_data(self):
        if self._bme_ok:
            import bme680
            try:
                sensor = bme680.BME680(bme680.I2C_ADDR_PRIMARY)
            except IOError:
                sensor = bme680.BME680(bme680.I2C_ADDR_SECONDARY)

            sensor.set_humidity_oversample(bme680.OS_2X)
            sensor.set_pressure_oversample(bme680.OS_4X)
            sensor.set_temperature_oversample(bme680.OS_8X)
            sensor.set_filter(bme680.FILTER_SIZE_3)

            self.temperature_bme = "{} C".format(sensor.data.temperature)
            self.pressure = "{} hPa".format(sensor.data.pressure)
            self.humidity = "{} %RH".format(sensor.data.humidity)
            self.air_quality = 'Not enough data!'

        if self._bno_ok:
            import time
            import board
            import busio
            import adafruit_bno055

            i2c = busio.I2C(board.SCL, board.SDA)
            sensor1 = adafruit_bno055.BNO055_I2C(i2c)
            self.temperature_bno = "{} C".format(sensor1.temperature)
            self.acceleration = "(m/s^2): {}".format(sensor1.acceleration)
            self.magnetic = "(microteslas): {}".format(sensor1.magnetic)
            self.gyro = "(rad/sec): {}".format(sensor1.gyro)
            self.euler = "{}".format(sensor1.euler)
            self.quaternion = "{}".format(sensor1.quaternion)
            self.linear_acceleration = "(m/s^2): {}".format(sensor1.linear_acceleration)
            self.gravity = "(m / s ^ 2): {}".format(sensor1.gravity)
        if self._si_ok:
            import time
            import SI1145.SI1145 as SI1145
            sensor2 = SI1145.SI1145()
            vis = sensor2.readVisible()
            IR = sensor.readIR()
            UV = sensor.readUV()
            uvIndex = (UV / 100.0)
            self.Vis = str(vis)
            self.IR = str(IR)
            self.UVindex = str(uvIndex)
