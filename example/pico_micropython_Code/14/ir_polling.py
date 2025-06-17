import time
from machine import Pin

class IRDecoder:
    def __init__(self, pin_no):
        self.pin = Pin(pin_no, Pin.IN, Pin.PULL_UP)
        self.last_code = None
        self.last_time = time.ticks_ms()
        self.signal_detected = False
        self.partial_data = 0
        self.bits_received = 0
        self.repeat_threshold = 100  

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
        if not self._wait_for_falling_edge(50000):
            return None
            
        t_start = time.ticks_us()
        if not self._wait_for_rising_edge(15000):
            return None
        t_low = time.ticks_diff(time.ticks_us(), t_start)

        if 8500 < t_low < 9500: 
            self.signal_detected = True
            self.bits_received = 0
            self.partial_data = 0
            
            if not self._wait_for_falling_edge(5000):
                return None
                
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
                    
            addr = raw & 0xFF
            addr_inv = (raw >> 8) & 0xFF
            cmd = (raw >> 16) & 0xFF
            cmd_inv = (raw >> 24) & 0xFF
            
            if cmd ^ cmd_inv == 0xFF:
                self.last_code = cmd
                self.last_time = time.ticks_ms()
                print(f"Full IR frame received: 0x{raw:08X}, cmd: 0x{cmd:02X}")
                return cmd
            else:
                print(f"Invalid IR checksum: 0x{raw:08X}")
                return None

        elif 2000 < t_low < 3000:
            if not self._wait_for_falling_edge(1000):
                return None
                
            current_time = time.ticks_ms()
            if (self.last_code is not None and 
                time.ticks_diff(current_time, self.last_time) > self.repeat_threshold):
                self.last_time = current_time
                print(f"Repeat IR code: 0x{self.last_code:02X}")
                return self.last_code
            else:
                return None
                
        elif 500 < t_low < 700:
            self.signal_detected = True
            self.bits_received += 1
            return None

        return None

