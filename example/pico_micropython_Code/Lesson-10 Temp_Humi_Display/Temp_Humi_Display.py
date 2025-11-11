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

from machine import Pin, I2C           # Import Pin and I2C classes for hardware control
from time import sleep, ticks_ms      # Import sleep and ticks_ms for timing functions
from dht20 import DHT20               # Import DHT20 class for temperature/humidity sensing
import ui                             # Import custom UI module for display updates

# === Variable Initialization ===
interval = 1000                       # Update interval: 1000 milliseconds (1 second)
last_update = 0                       # Initialize last update timestamp

# Use I2C1 controller with GP2 (SDA) and GP3 (SCL) pins
i2c = I2C(1, scl=Pin(3), sda=Pin(2))  # Initialize I2C bus with specified pins
sensor = DHT20(i2c)                   # Create DHT20 sensor object on I2C bus

while True:                           # Main loop runs indefinitely
    current = ticks_ms()              # Get current time in milliseconds

    if current - last_update >= interval:  # Check if update interval has elapsed
        try:
            temp, humi = sensor.measure()  # Read temperature and humidity from sensor
            
            # Update UI labels with formatted values
            ui.ui_TempLabel.set_text("Temperature:  " + "{:.1f} C".format(temp))
            ui.ui_HumiLabel.set_text("Humidity:  " + "{:.1f} %".format(humi))

            # Print values to console with higher precision
            print("Temperature: {:.2f} °C".format(temp))
            print("Humidity: {:.2f} %".format(humi))

        except Exception as e:        # Handle exceptions during sensor reading
            print("Failed to read DHT20:", e)  # Print error message

        last_update = current         # Update last update timestamp

    sleep(0.1)                        # Short delay to reduce CPU usage


