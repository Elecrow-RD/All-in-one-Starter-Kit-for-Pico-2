from machine import Pin, ADC, PWM
import time

# === Hardware Configuration ===
RHEO_PIN     = 28   # Rheostat connected to GP28 (ADC2 channel)
RED_PIN      = 18   # Red LED connected to GPIO18
YELLOW_PIN   = 20   # Yellow LED connected to GPIO20
GREEN_PIN    = 19   # Green LED connected to GPIO19
PWM_FREQ     = 1000 # PWM signal frequency (Hz), controls LED flicker rate

# === Rheostat Class: Handles potentiometer input ===
class Rheostat:
    def __init__(self, pin):
        self.adc = ADC(pin)  # Initialize ADC (Analog-to-Digital Converter) for specified pin

    def read_raw(self):
        return self.adc.read_u16()  # Read raw ADC value (16-bit unsigned integer, range 0-65535)

    def read_normalized(self):
        return self.read_raw() / 65535  # Normalize raw value to 0.0-1.0 range

    def read_duty(self):
        return int(self.read_normalized() * 65535)  # Convert normalized value to PWM duty cycle (0-65535)

# === Initialize potentiometer and three PWM outputs ===
rheo = Rheostat(RHEO_PIN)  # Create potentiometer object

# Initialize PWM control for three LEDs
p_red    = PWM(Pin(RED_PIN))
p_yellow = PWM(Pin(YELLOW_PIN))
p_green  = PWM(Pin(GREEN_PIN))

# Configure frequency for all PWM outputs and initialize to OFF state
for p in (p_red, p_yellow, p_green):
    p.freq(PWM_FREQ)
    p.duty_u16(0)  # Set duty cycle to 0% (LED off)

# === Main loop: Control LED brightness based on potentiometer position ===
while True:
    # Read PWM duty cycle value corresponding to current potentiometer position
    duty = rheo.read_duty()  # Range: 0 (fully off) - 65535 (fully on)

    # Synchronously control brightness of all three LEDs
    p_red.duty_u16(duty)
    p_yellow.duty_u16(duty)
    p_green.duty_u16(duty)

    # Print debug information (raw potentiometer value and PWM duty cycle)
    print("Potentiometer ADC:", rheo.read_raw(), "// PWM Duty Cycle:", duty)

    time.sleep(0.1)  # Delay 100ms to control sampling frequency and reduce CPU usage