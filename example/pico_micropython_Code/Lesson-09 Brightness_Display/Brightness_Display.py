from st77xx import *
import ft6x36
import machine
import fs_driver
import sys
import lvgl as lv
sys.path.append('.')

spi=machine.SPI(
    0,
    baudrate=24_000_000,
    polarity=0,
    phase=0,
    sck=machine.Pin(6, machine.Pin.OUT),
    mosi=machine.Pin(7, machine.Pin.OUT),
    miso=None
)
# dma=rp2_dma.DMA(0)
rp2_dma=None

lcd =St7789(rot=1, res=(240,320), spi=spi,rp2_dma=rp2_dma,cs=17,dc=16,bl=0,bgr=True,rst=None)

# bl
p0 = machine.Pin(0, machine.Pin.OUT)
p0(1)

i2c = machine.I2C(0, scl=machine.Pin(5), sda=machine.Pin(4))
touch = ft6x36.FT6x36()

fs_drv = lv.fs_drv_t()
fs_driver.fs_register(fs_drv, 'S')

# =================================================================================

from machine import Pin, ADC, PWM  # Import Pin, ADC, and PWM classes for hardware control
from time import sleep              # Import sleep function for timing
import ui                           # Import custom UI module for display updates

# === Pin Definitions ===
slider_adc = ADC(Pin(28))  # Initialize ADC on pin 28 for analog slider input
red_led = PWM(Pin(18))     # Initialize red LED with PWM on pin 18
yellow_led = PWM(Pin(20))  # Initialize yellow LED with PWM on pin 20
green_led = PWM(Pin(19))   # Initialize green LED with PWM on pin 19

# Set PWM frequency (similar to analogWrite in Arduino)
for led in [red_led, yellow_led, green_led]:
    led.freq(1000)  # Set PWM frequency to 1000Hz for all LEDs

last_level = -1  # Track the previous brightness level (initialize to invalid value)

def update_leds(pwm_val):
    # Set PWM duty cycle for three LEDs (0~65535 range)
    duty = int(pwm_val / 255 * 65535)  # Convert 0-255 range to 0-65535
    red_led.duty_u16(duty)            # Set red LED duty cycle
    yellow_led.duty_u16(duty)         # Set yellow LED duty cycle
    green_led.duty_u16(duty)          # Set green LED duty cycle

def get_level(analog_val):
    adc_min = 200                    # Minimum valid ADC value (calibration)
    adc_max = 65300                  # Maximum valid ADC value (calibration)
    if analog_val < adc_min:
        analog_val = adc_min         # Cap value at minimum
    elif analog_val > adc_max:
        analog_val = adc_max         # Cap value at maximum

    normalized = (analog_val - adc_min) / (adc_max - adc_min)  # Normalize to 0-1
    level = int(normalized * 11)     # Map to 0-10 brightness levels (11 steps)

    return min(level, 10)            # Ensure level does not exceed 10

while True:
    analog_val = slider_adc.read_u16()  # Read raw 16-bit ADC value (0-65535)

    pwm_val = int(analog_val / 65535 * 255)  # Map to 0-255 PWM range
    pwm_val = min(max(pwm_val, 0), 255)     # Clamp to valid range

    level = get_level(analog_val)  # Get calibrated brightness level (0-10)

    update_leds(pwm_val)           # Update LED brightness via PWM

    if level != last_level:        # Check if brightness level changed
        last_level = level         # Update last recorded level
        try:
            ui.ui_Label1.set_text("Brightness: " + str(level))  # Update UI label
        except Exception as e:
            print("LVGL label1 update failed:", e)  # Print error on update failure

        print("ADC:", analog_val, "| PWM:", pwm_val, "| Level:", level)  # Debug output

    sleep(0.1)                     # Sleep for 0.1s to reduce CPU usage



            