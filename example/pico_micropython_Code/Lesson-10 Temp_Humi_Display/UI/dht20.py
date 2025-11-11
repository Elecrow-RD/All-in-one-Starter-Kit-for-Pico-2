import time

class DHT20:
    def __init__(self, i2c, address=0x38):
        self.i2c = i2c
        self.address = address
        self._init_sensor()

    def _init_sensor(self):
        # 初始化传感器
        try:
            self.i2c.writeto(self.address, b'\x71')
            time.sleep(0.01)
        except:
            pass

    def _read_data(self):
        self.i2c.writeto(self.address, b'\xAC\x33\x00')
        time.sleep(0.08)
        for _ in range(10):
            time.sleep(0.01)
            data = self.i2c.readfrom(self.address, 7)
            if (data[0] & 0x80) == 0:
                break
        else:
            raise Exception("Sensor timeout")

        return data

    def measure(self):
        data = self._read_data()

        humidity_raw = ((data[1] << 12) | (data[2] << 4) | (data[3] >> 4))
        temperature_raw = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5]

        humidity = (humidity_raw / (1 << 20)) * 100
        temperature = (temperature_raw / (1 << 20)) * 200 - 50

        return temperature, humidity
