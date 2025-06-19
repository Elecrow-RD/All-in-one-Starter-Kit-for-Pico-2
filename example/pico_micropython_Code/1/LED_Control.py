# Import the Pin class to control GPIO pins
from machine import Pin
# Import the time module to implement delay functionality
import time

# Define the GPIO pins connected to the LEDs and set them to output mode
red_led = Pin(18, Pin.OUT)    # Red LED connected to GPIO18
yellow_led = Pin(20, Pin.OUT) # Yellow LED connected to GPIO20
green_led = Pin(19, Pin.OUT)  # Green LED connected to GPIO19

# Define the time interval (in milliseconds) for LED state switching
interval = 1000
# Record the timestamp of the last LED state update
last_time = time.ticks_ms()

# Initialization function: Turn off all LEDs
def L1_Init():
    red_led.off()    # Turn off the red LED
    yellow_led.off() # Turn off the yellow LED
    green_led.off()  # Turn off the green LED

# Main control function: Implement LED blinking at intervals
def L1_Curriculum():
    global last_time  # Declare the use of the global variable last_time
    L1_Init()         # Initialize all LEDs to the off state
    
    # Infinite loop to continuously control LED blinking
    while True:
        # Get the current system time (in milliseconds)
        current_time = time.ticks_ms()
        # Check if the set time interval has been reached
        if time.ticks_diff(current_time, last_time) >= interval:
            last_time = current_time  # Update the last update time to the current time
            # Reverse the state of each LED (on → off, off → on)
            red_led.value(not red_led.value())
            yellow_led.value(not yellow_led.value())
            green_led.value(not green_led.value())

# Program entry point: Call the main control function to start LED blinking
L1_Curriculum()