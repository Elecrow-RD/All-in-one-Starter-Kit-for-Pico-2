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

import time  # Import the time module for timing functions
from ir_polling import IRDecoder  # Import the IRDecoder class for infrared signal decoding
from machine import Pin  # Import the Pin class from the machine module
from machine import PWM  # Import the PWM class from the machine module
import ui  # Import the ui module for updating the user interface
from servo import Servo  # Import the Servo class for controlling a servo motor

# Infrared remote control code mapping table
KEY_CODES = {
    0x45: "CH-",  # Channel down
    0x46: "CH",   # Channel
    0x47: "CH+",  # Channel up
    0x44: "PREV",         # Stop the servo
    0x40: "NEXT",         # Start the servo
    0x43: "PLAY/PAUSE",   # Confirm input angle
    0x07: "VOL-",
    0x15: "VOL+",
    0x09: "EQ",
    0x16: "0",
    0x19: "100+",
    0x0D: "200+",
    0x0C: "1",
    0x18: "2",
    0x5E: "3",
    0x08: "4",
    0x1C: "5",
    0x5A: "6",
    0x42: "7",
    0x52: "8",
    0x4A: "9"
}

decoder = IRDecoder(pin_no=11)  # Initialize the IR decoder on pin 11
SERVO_PIN = 13  # Define the pin number for the servo motor
servo = Servo(pin_num=SERVO_PIN)  # Initialize the servo motor on the defined pin

ir_enabled = True  # Flag to enable/disable IR control
servo_running = False  # Flag to indicate if the servo is running
angle_input = ""  # String to store the input angle from the remote control
current_angle = 0  # Current angle of the servo motor

last_key = None  # Store the last key code received
last_time = 0  # Store the last time a key was pressed
key_pressed = False  # Flag to indicate if a key is currently pressed
key_release_time = 0  # Store the time when the key was released
MIN_RELEASE_TIME = 50  # Minimum time between key releases to prevent multiple presses

# Function to update the UI label
def update_label(text):
    ui.ui_Label1.set_text(text)  # Update the text of the UI label

# Function to start the servo motor
def start_servo():
    global servo_running
    servo_running = True  # Set the flag to indicate the servo is running
    update_label(f"Servo started at {current_angle}°")  # Update the UI label

# Function to stop the servo motor
def stop_servo():
    global servo_running
    servo_running = False  # Set the flag to indicate the servo is stopped
    update_label("Servo stopped")  # Update the UI label

# Function to move the servo motor to a specified angle
def move_servo(angle):
    global current_angle
    current_angle = max(0, min(180, angle))  # Ensure the angle is within the valid range (0-180)
    servo.write_angle(current_angle)  # Set the servo to the specified angle
    update_label(f"Angle set to {current_angle}°")  # Update the UI label

try:
    while True:  # Main loop
        # Read the infrared signal
        key_code = decoder.read()  # Get the key code from the IR decoder
        now = time.ticks_ms()  # Get the current time in milliseconds
        
        # Handle key release state
        if key_code is None and key_pressed:  # Check if no key is pressed and a key was previously pressed
            key_release_time = now  # Update the key release time
            key_pressed = False  # Reset the key pressed flag
        
        # Handle new key press
        if key_code is not None and ir_enabled:  # Check if a key code is received and IR control is enabled
            # Check if it's a new key press
            is_new_key = (key_code != last_key) or (  # Check if the key code is different from the last one
                key_code == last_key and  # Check if the same key is pressed again
                not key_pressed and  # Check if no key is currently pressed
                time.ticks_diff(now, key_release_time) > MIN_RELEASE_TIME and  # Check if enough time has passed since the last release
                time.ticks_diff(now, last_time) > 200  # Check if enough time has passed since the last press (debounce)
            )
            
            if is_new_key:  # If it's a new key press
                last_key = key_code  # Update the last key code
                last_time = now  # Update the last press time
                key_pressed = True  # Set the key pressed flag
                
                key = KEY_CODES.get(key_code, f"Unknown(0x{key_code:02X})")  # Get the key name from the mapping table
                print("Received:", key)  # Print the received key
                
                # Key handling logic
                if key == "CH-":  # Disable IR control
                    ir_enabled = False
                    update_label("IR Disabled")
                    angle_input = ""
                    continue

                if key == "CH+":  # Enable IR control
                    ir_enabled = True
                    update_label("IR Enabled")
                    angle_input = ""
                    continue

                if key.isdigit():  # Check if the key is a digit
                    if len(angle_input) < 3:  # Check if the input angle string is less than 3 characters
                        angle_input += key  # Append the digit to the input angle string
                        update_label(f"Input Angle: {angle_input}")  # Update the UI label
                    continue

                if key == "PLAY/PAUSE":  # Confirm input angle
                    if angle_input != "":  # Check if there is an input angle
                        angle_val = int(angle_input)  # Convert the input angle string to an integer
                        move_servo(angle_val)  # Move the servo to the specified angle
                        angle_input = ""  # Reset the input angle string
                        start_servo()  # Start the servo motor
                    else:
                        update_label("No angle input")  # Update the UI label if no angle is input
                    continue

                if key == "PREV":  # Stop the servo motor
                    stop_servo()
                    continue

                if key == "NEXT":  # Start the servo motor
                    start_servo()
                    move_servo(current_angle)  # Move the servo to the current angle
                    continue
        
        # Regularly update the UI and handle other tasks
        lv.task_handler()  # Handle LVGL tasks to keep the UI responsive
        time.sleep_ms(10)  # Small delay to ensure timely response

except KeyboardInterrupt:  # Handle program termination
    servo.deinit()  # Deinitialize the servo motor
    print("Program stopped")  # Print a message indicating the program has stopped
