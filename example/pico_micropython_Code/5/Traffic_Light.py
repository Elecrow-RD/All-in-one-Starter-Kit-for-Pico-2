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
import time                # Import time module for sleep and time-related functions
from machine import Pin     # Import Pin class from machine module for GPIO control

# Tri-color LED pin definitions (based on provided configuration)
RED_LED = Pin(18, Pin.OUT)         # Initialize red LED on pin 18 as output
YELLOW_LED = Pin(20, Pin.OUT)      # Initialize yellow LED on pin 20 as output
GREEN_LED = Pin(19, Pin.OUT)       # Initialize green LED on pin 19 as output

# Stage enumeration
GREEN_STAGE = 0                  # Define green stage (0)
YELLOW_STAGE = 1                 # Define yellow stage (1)
RED_STAGE = 2                    # Define red stage (2)

# Initial state variables
current_stage = GREEN_STAGE      # Set initial stage to green
remaining_time = 30              # Set initial countdown time to 30 seconds
blink_state = False              # Initialize blink state (off)
last_blink_time = time.ticks_ms() # Record initial time for blink timing

# Initial display setup
ui.ui_Label1.set_text("Countdown: 30s")  # Set initial label text to show 30s countdown
GREEN_LED.value(1)                     # Turn on green LED initially

while True:                            # Enter infinite main loop
    # Update countdown display
    ui.ui_Label1.set_text("Countdown: {:2d}s".format(remaining_time))  # Update label with current time
    time.sleep(1)                      # Wait for 1 second
    remaining_time -= 1                # Decrement remaining time by 1 second

    # Blinking logic (last 5 seconds)
    if 0 < remaining_time <= 5:        # Check if remaining time is between 1-5 seconds
        now = time.ticks_ms()          # Get current time in milliseconds
        if time.ticks_diff(now, last_blink_time) >= 500:  # Check if 500ms has passed
            blink_state = not blink_state  # Toggle blink state
            last_blink_time = now        # Update last blink time
            if current_stage == GREEN_STAGE:  # If in green stage
                GREEN_LED.value(blink_state)  # Set green LED to blink state
            elif current_stage == RED_STAGE:  # If in red stage
                RED_LED.value(blink_state)    # Set red LED to blink state
    else:                                # If not in last 5 seconds
        # Steady-on logic
        if current_stage == GREEN_STAGE:  # If in green stage
            GREEN_LED.value(1)           # Turn on green LED
        elif current_stage == YELLOW_STAGE:  # If in yellow stage
            YELLOW_LED.value(1)          # Turn on yellow LED
        elif current_stage == RED_STAGE:  # If in red stage
            RED_LED.value(1)             # Turn on red LED

    # Stage transition logic
    if remaining_time == 0:            # When countdown reaches 0
        # Turn off all LEDs
        RED_LED.value(0)               # Turn off red LED
        YELLOW_LED.value(0)            # Turn off yellow LED
        GREEN_LED.value(0)             # Turn off green LED

        if current_stage == GREEN_STAGE:  # If transitioning from green stage
            current_stage = YELLOW_STAGE  # Move to yellow stage
            remaining_time = 3           # Set yellow stage duration to 3s
            YELLOW_LED.value(1)          # Turn on yellow LED
        elif current_stage == YELLOW_STAGE:  # If transitioning from yellow stage
            current_stage = RED_STAGE    # Move to red stage
            remaining_time = 20          # Set red stage duration to 20s
            RED_LED.value(1)             # Turn on red LED
        elif current_stage == RED_STAGE:  # If transitioning from red stage
            current_stage = GREEN_STAGE  # Reset to green stage
            remaining_time = 30          # Set green stage duration to 30s
            GREEN_LED.value(1)           # Turn on green LED

