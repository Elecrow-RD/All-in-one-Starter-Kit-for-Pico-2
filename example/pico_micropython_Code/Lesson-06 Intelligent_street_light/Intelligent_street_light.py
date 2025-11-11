from machine import I2C, Pin  # Import I2C and Pin modules for hardware communication and pin control
import time  # Import time module for delay operations

class LightSensor:  # Define light sensor class
    def __init__(self, scl_pin=3, sda_pin=2, addr=0x5C):  # Initialization method to set I2C communication parameters
        self.i2c = I2C(1, scl=scl_pin, sda=sda_pin, freq=400000)  # Create an I2C object using bus 1, set SCL/SDA pins and communication frequency
        self.addr = addr  # Store the sensor's I2C address
        self.i2c.writeto(self.addr, b'\x10')  # Send a command to the sensor to start high-precision continuous measurement mode

    def get_lux(self):  # Define method to obtain light intensity value
        data = self.i2c.readfrom(self.addr, 2)  # Read 2 bytes of data from the sensor
        raw = (data[0] << 8) | data[1]  # Combine two bytes into a 16-bit integer
        return raw / 1.2  # Convert raw data to standard light intensity unit Lux according to the sensor specification

# Hardware configuration area
LED_PIN   = 20     # Define the GPIO pin number connected to the LED
THRESHOLD = 100    # Set the light intensity threshold in Lux, used as the critical value to control LED on/off

sensor = LightSensor()  # Create a light sensor object
led = Pin(LED_PIN, Pin.OUT)  # Set the LED pin to output mode for controlling LED on/off

try:  # Start exception handling
    while True:  # Infinite loop to continuously monitor light and control the LED
        lux = sensor.get_lux()  # Call the sensor object's method to get the current light intensity value
        if lux < THRESHOLD:  # Check if the light intensity value is below the threshold
            led.on()  # If below the threshold, turn on the LED (output high level)
        else:  # If the light intensity value is above the threshold
            led.off() # Turn off the LED (output low level)
        print(f"Light: {lux:.1f} Lux | LED Status: {'ON' if led.value() else 'OFF'}")  # Print the current light intensity value and LED status
        time.sleep(0.5)  # Delay for 0.5 seconds to control the detection frequency and avoid excessive frequency

except KeyboardInterrupt:  # Catch keyboard interrupt exception (Ctrl+C)
    led.off()  # Ensure the LED is turned off before the program terminates
    print("Program stopped, LED turned off")  # Prompt the user that the program has stopped