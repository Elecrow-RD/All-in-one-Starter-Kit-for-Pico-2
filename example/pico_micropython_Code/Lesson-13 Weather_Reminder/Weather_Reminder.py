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

from machine import Pin, I2C, PWM  # Import Pin, I2C, and PWM classes from machine module
from time import sleep, ticks_ms    # Import sleep and ticks_ms functions from time module
from dht20 import DHT20            # Import DHT20 class from dht20 module
import ui                          # Import the custom ui module for interface updates

# === Hardware Pin Configuration ===
red_led = Pin(18, Pin.OUT)         # Initialize red LED on pin 18 as output
yellow_led = Pin(20, Pin.OUT)      # Initialize yellow LED on pin 20 as output
buzzer = PWM(Pin(10))              # Initialize buzzer on pin 10 with PWM control

# === Time Management ===
interval = 1000                    # Set data update interval to 1000ms (1 second)
last_update = 0                    # Initialize last update timestamp

# === I2C Initialization ===
i2c = I2C(1, scl=Pin(3), sda=Pin(2))  # Initialize I2C bus 1 with SCL on pin 3 and SDA on pin 2
sensor = DHT20(i2c)                  # Create DHT20 sensor object connected via I2C

# === Main Loop ===
while True:                          # Enter infinite loop
    current = ticks_ms()             # Get current time in milliseconds
                                            
    if current - last_update >= interval:  # Check if update interval has elapsed
        try:                           # Try to execute sensor reading and processing
            temp, humi = sensor.measure()  # Read temperature and humidity from DHT20
                                            
            # Update UI display elements
            ui.ui_TempLabel.set_text("{:.1f} C".format(temp))  # Set temperature label text
            ui.ui_HumiLabel.set_text("{:.1f} %".format(humi))  # Set humidity label text
                                            
            print("Temperature: {:.1f} °C".format(temp))  # Print temperature to console
            print("Humidity: {:.1f} %".format(humi))      # Print humidity to console
                                            
            # Control LED indicators based on temperature
            if temp > 30:                # If temperature > 30°C
                red_led.on()             # Turn on red LED
                yellow_led.off()         # Turn off yellow LED
                ui.ui_AlarmLabel.set_text("It's hot")  # Set alarm label
            elif temp > 25:              # If temperature > 25°C (but ≤30°C)
                yellow_led.on()          # Turn on yellow LED
                red_led.off()            # Turn off red LED
                ui.ui_AlarmLabel.set_text("Temperature is high")  # Set alarm label
            else:                        # If temperature ≤25°C
                red_led.off()            # Turn off red LED
                yellow_led.off()         # Turn off yellow LED
                                            
            # Humidity alarm (control buzzer)
            if humi < 40.0:              # If humidity < 40%
                buzzer.freq(1300)        # Set buzzer frequency to 1300Hz
                buzzer.duty_u16(30000)   # Set buzzer duty cycle (turn on)
                ui.ui_AlarmLabel.set_text("Air is dry")  # Set alarm label
            else:                        # If humidity ≥40%
                buzzer.duty_u16(0)       # Set buzzer duty cycle to 0 (turn off)
                if temp <= 25 and humi >= 40.0:  # If normal temp and humidity
                    ui.ui_AlarmLabel.set_text("")  # Clear alarm label
                                            
        except Exception as e:           # Catch any exceptions during sensor reading
            print("Failed to read DHT20:", e)  # Print error to console
                                            
        last_update = current            # Update last update timestamp to current time

    sleep(0.1)                           # Sleep for 0.1 seconds to reduce CPU load