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

# ===========================================================================================

import ui                  # Import the custom UI module for interface control
import time                # Import time module for sleep functionality
import machine             # Import machine module for hardware control

# 1. Display "HELLO WORLD!"
ui.ui_Label1.set_text("HELLO WORLD!")  # Set text of ui_Label1 to "HELLO WORLD!"
time.sleep(1)                          # Pause execution for 1 second

# 2. Display "Bye Bye"
ui.ui_Label1.set_text("Bye Bye")       # Update label text to "Bye Bye"
time.sleep(1)                          # Pause for 1 second

# 3. Clear the screen and turn off backlight
ui.ui_Label1.set_text("")              # Clear the label text (empty string)

print("OVER")                          # Print "OVER" to the console
# Backlight is connected to GPIO 0
backlight = machine.Pin(0, machine.Pin.OUT)  # Initialize GPIO 0 as output for backlight control
backlight.value(0)  # Turn off the backlight (set pin to low)  # Set backlight pin to low (off state)
