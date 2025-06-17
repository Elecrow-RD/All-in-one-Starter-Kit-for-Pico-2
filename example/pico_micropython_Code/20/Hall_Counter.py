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

import machine  # Import the machine module for hardware control
import time  # Import the time module for timing functions
import ui  # Import the ui module for updating the user interface

# Pin definitions
hall_pin = machine.Pin(21, machine.Pin.IN)  # Define the Hall sensor pin as an input
red_led = machine.Pin(18, machine.Pin.OUT)  # Define the red LED pin as an output

count = 0  # Initialize the counter to zero

# Function to update the UI label
def update_label(text):
    ui.ui_Label1.set_text(text)  # Update the text of the UI label

# Initialize the display with the initial counter value
update_label("counter: {}".format(count))  # Set the initial message on the UI label

# Main loop
while True:
    if hall_pin.value() == 0:  # Check if the Hall sensor is triggered (magnetic field detected)
        time.sleep_ms(50)  # Debounce delay to avoid false triggers
        if hall_pin.value() == 0:  # Re-check to ensure the sensor is still triggered
            # Wait for the magnet to move away to prevent multiple counts
            while hall_pin.value() == 0:
                time.sleep_ms(10)  # Short delay to wait for the magnet to move away

            count += 1  # Increment the counter
            update_label("counter: {}".format(count))  # Update the UI label with the new counter value

            red_led.value(1)  # Turn on the red LED
            time.sleep_ms(200)  # Keep the LED on for 200 milliseconds
            red_led.value(0)  # Turn off the red LED

    time.sleep_ms(10)  # Short delay to avoid too frequent checks
