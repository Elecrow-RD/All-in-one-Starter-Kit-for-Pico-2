# Import asynchronous I/O library, using uasyncio as the MicroPython asynchronous implementation
import uasyncio as asyncio
# Import the Pin class for controlling GPIO pins
from machine import Pin
# Import the NEC protocol infrared receiver class
from ir_rx.nec import NEC_8
# Import the error printing function
from ir_rx.print_error import print_error
# Import thread-safe flag for communication between asynchronous tasks
from uasyncio import ThreadSafeFlag

# Define LED control pins
red_led = Pin(18, Pin.OUT)    # Red LED connected to GPIO18
yellow_led = Pin(20, Pin.OUT) # Yellow LED connected to GPIO20
green_led = Pin(19, Pin.OUT)  # Green LED connected to GPIO19

# Define the mapping from infrared remote control key codes to key names
KEY_CODES = {
    0x0C: "1",  # Infrared code corresponding to button 1
    0x18: "2",  # Infrared code corresponding to button 2
    0x5E: "3",  # Infrared code corresponding to button 3
    0x08: "4",  # Infrared code corresponding to button 4
    0x1C: "5",  # Infrared code corresponding to button 5
    0x5A: "6",  # Infrared code corresponding to button 6
}

# The currently running asynchronous task
current_task = None

# Create a thread-safe flag to notify the main loop of new key inputs
flag = ThreadSafeFlag()
# Store the received key value for processing by the main loop
received_key = None

# Helper function to turn off all LEDs
def turn_off_all():
    red_led.off()     # Turn off the red LED
    yellow_led.off()  # Turn off the yellow LED
    green_led.off()   # Turn off the green LED

# Asynchronous function: Control all three LEDs to flash simultaneously
async def blink_all():
    try:
        while True:
            # Turn on all LEDs
            red_led.on()
            yellow_led.on()
            green_led.on()
            await asyncio.sleep(0.3)  # Maintain for 0.3 seconds
            
            # Turn off all LEDs
            red_led.off()
            yellow_led.off()
            green_led.off()
            await asyncio.sleep(0.3)  # Maintain for 0.3 seconds
            
    except asyncio.CancelledError:
        # Ensure all LEDs are turned off when the task is cancelled
        turn_off_all()
        raise  # Re-throw the exception to notify task cancellation

# Asynchronous function: Implement a running light effect with red-yellow-green LEDs cycling
async def running_light():
    try:
        while True:
            # Turn on the red light
            red_led.on()
            yellow_led.off()
            green_led.off()
            await asyncio.sleep(0.3)  # Maintain for 0.3 seconds
            
            # Turn on the yellow light
            red_led.off()
            yellow_led.on()
            green_led.off()
            await asyncio.sleep(0.3)  # Maintain for 0.3 seconds
            
            # Turn on the green light
            red_led.off()
            yellow_led.off()
            green_led.on()
            await asyncio.sleep(0.3)  # Maintain for 0.3 seconds
            
    except asyncio.CancelledError:
        # Ensure all LEDs are turned off when the task is cancelled
        turn_off_all()
        raise  # Re-throw the exception to notify task cancellation

# Infrared reception callback function, called by interrupt when an infrared signal is received
def ir_callback(data, addr, ctrl):
    global received_key
    if data < 0:
        # Negative values indicate repeated keys, ignore
        return
    
    # Look up the corresponding key name based on the received infrared code
    key = KEY_CODES.get(data, None)
    if key:
        print(f"Key received: {key}")
        received_key = key
        flag.set()  # Set the flag to wake up the waiting main loop

# Initialize the infrared receiver
ir_pin = Pin(11, Pin.IN)  # Infrared receiver connected to GPIO11
ir = NEC_8(ir_pin, ir_callback)  # Create an NEC protocol infrared receiver instance
ir.error_function(print_error)  # Set the error handling function

# Main program asynchronous task, responsible for handling key events and controlling LEDs
async def main():
    global current_task
    while True:
        await flag.wait()  # Wait for an infrared key event
        
        # Get the received key value
        key = received_key
        
        # Cancel the currently running task if any
        if current_task:
            current_task.cancel()
            try:
                await current_task  # Wait for the task to cancel completely
            except asyncio.CancelledError:
                pass  # Ignore cancellation exceptions
            current_task = None
        
        # Execute corresponding operations based on the key value
        if key == "1":
            turn_off_all()  # Turn off all LEDs
            red_led.on()    # Turn on the red LED
            print("Red light on")
        elif key == "2":
            turn_off_all()    # Turn off all LEDs
            yellow_led.on()   # Turn on the yellow LED
            print("Yellow light on")
        elif key == "3":
            turn_off_all()    # Turn off all LEDs
            green_led.on()    # Turn on the green LED
            print("Green light on")
        elif key == "4":
            print("Start blinking")
            # Create and start the LED blinking task
            current_task = asyncio.create_task(blink_all())
        elif key == "5":
            print("Start running light")
            # Create and start the running light task
            current_task = asyncio.create_task(running_light())
        elif key == "6":
            turn_off_all()    # Turn off all LEDs
            print("All lights off")
            current_task = None  # Ensure no active tasks

# Program entry point
try:
    asyncio.run(main())  # Start the main event loop
except KeyboardInterrupt:
    # Capture Ctrl+C interrupt signal
    if current_task:
        current_task.cancel()  # Cancel the running task
    ir.close()  # Close the infrared receiver
    print("Program terminated")