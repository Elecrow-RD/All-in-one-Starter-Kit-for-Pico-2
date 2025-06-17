# ir_polling.py
import time
from machine import Pin

class IRDecoder:
    def __init__(self, pin_no):
        self.pin = Pin(pin_no, Pin.IN, Pin.PULL_UP)
        self.last_code = None
        self.last_time = time.ticks_ms()

    def _wait_for_falling_edge(self, timeout_us):
        t0 = time.ticks_us()
        while self.pin.value() == 1:
            if time.ticks_diff(time.ticks_us(), t0) > timeout_us:
                return False
        return True

    def _wait_for_rising_edge(self, timeout_us):
        t0 = time.ticks_us()
        while self.pin.value() == 0:
            if time.ticks_diff(time.ticks_us(), t0) > timeout_us:
                return False
        return True

    def read(self):
        if not self._wait_for_falling_edge(100000):  # 等待起始低电平
            return None
        t_start = time.ticks_us()
        if not self._wait_for_rising_edge(10000):  # 起始低电平结束
            return None
        t_low = time.ticks_diff(time.ticks_us(), t_start)

        if t_low > 8500:  # 起始位
            # 检测4.5ms高电平
            if not self._wait_for_falling_edge(5000):
                return None
            # 读取32位数据
            raw = 0
            for i in range(32):
                if not self._wait_for_rising_edge(1000):
                    return None
                t_high_start = time.ticks_us()
                if not self._wait_for_falling_edge(2000):
                    return None
                pulse_width = time.ticks_diff(time.ticks_us(), t_high_start)
                raw >>= 1
                if pulse_width > 1000:
                    raw |= 0x80000000
            # 拆分地址、地址反码、命令、命令反码
            addr = raw & 0xFF
            addr_inv = (raw >> 8) & 0xFF
            cmd = (raw >> 16) & 0xFF
            cmd_inv = (raw >> 24) & 0xFF
            if cmd ^ cmd_inv == 0xFF:
                self.last_code = cmd
                self.last_time = time.ticks_ms()
                return cmd
            else:
                return None

        elif t_low > 2000:
            return None

        return None



