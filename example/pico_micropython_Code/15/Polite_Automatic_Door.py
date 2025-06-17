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

from machine import Pin  # Import the Pin class from the machine module
import time  # Import the time module for delay functions
import ui  # Import the ui module for updating the user interface

# Constant definitions
RELAY_PIN = 12      # Define the pin number connected to the relay
TOUCH_PIN = 14      # Define the pin number connected to the touch sensor

# Initialize pins
relay = Pin(RELAY_PIN, Pin.OUT)  # Initialize the relay pin as an output
touch = Pin(TOUCH_PIN, Pin.IN)   # Initialize the touch sensor pin as an input

# Initialization
relay.value(0)      # Set the initial state of the relay to off

# Function to update the UI label
def update_label(text):
    ui.ui_Label1.set_text(text)  # Update the text of the UI label

# Display the initial message
update_label("Please open the door")  # Set the initial message on the UI label

try:
    while True:  # Main loop
        # Check if the touch button is pressed
        if touch.value():  # If the touch sensor is triggered
            # Open the door: activate the relay
            relay.value(1)  # Turn on the relay
            update_label("Welcome")  # Display a welcome message
            
            # Wait for 10 seconds
            time.sleep(10)
            
            # Close the door: deactivate the relay
            relay.value(0)  # Turn off the relay
            update_label("Please open the door")  # Reset the initial message
            
            # Short delay to prevent multiple triggers
            time.sleep(0.5)
        
        # Short delay to avoid too frequent checks
        time.sleep(0.1)
        
        # Handle LVGL tasks
        lv.task_handler()  # Process LVGL tasks to keep the UI responsive

except KeyboardInterrupt:
    # Clean up resources
    # (Resource cleanup code would go here, but it's not implemented in this snippet)
    relay.value(0)
    print("Program stopped")


