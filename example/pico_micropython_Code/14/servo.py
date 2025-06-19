from machine import Pin, PWM
import time

class Servo:
    def __init__(self, pin_num, freq=50, min_us=600, max_us=2400):
        self.pwm = PWM(Pin(pin_num))
        self.pwm.freq(freq)
        self.min_us = min_us
        self.max_us = max_us
        self.period = 1000000 // freq

    def _angle_to_duty(self, angle):
        if angle < 0:
            angle = 0
        elif angle > 180:
            angle = 180
        us = self.min_us + (self.max_us - self.min_us) * (angle / 180)
        duty = int((us / self.period) * 65535)
        return duty

    def write_angle(self, angle):
        duty = self._angle_to_duty(angle)
        self.pwm.duty_u16(duty)
        time.sleep_ms(50)

    def deinit(self):
        self.pwm.deinit()

