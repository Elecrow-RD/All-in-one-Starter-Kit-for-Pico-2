from machine import I2C
import time

class LSM6DS3TRC:
    def __init__(self, i2c, addr=0x6B):
        self.i2c = i2c
        self.addr = addr
        self.init_sensor()

    def write_register(self, reg, data):
        self.i2c.writeto_mem(self.addr, reg, bytes([data]))

    def read_register(self, reg, length):
        return self.i2c.readfrom_mem(self.addr, reg, length)

    def init_sensor(self):
        self.write_register(0x10, 0b01000000)  # CTRL1_XL: ODR_XL = 104Hz, FS_XL = ±2g
        self.write_register(0x12, 0b01000100)  # CTRL3_C: BDU=1, IF_INC=1
        time.sleep(0.1)

    def twos_complement(self, val):
        return val - 65536 if val & 0x8000 else val

    def read_acceleration(self):
        raw = self.read_register(0x28, 6)
        x = self.twos_complement(raw[1] << 8 | raw[0]) * 0.061 / 1000
        y = self.twos_complement(raw[3] << 8 | raw[2]) * 0.061 / 1000
        z = self.twos_complement(raw[5] << 8 | raw[4]) * 0.061 / 1000
        return x, y, z
