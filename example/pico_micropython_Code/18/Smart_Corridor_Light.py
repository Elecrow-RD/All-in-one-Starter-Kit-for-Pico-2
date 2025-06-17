from machine import Pin, ADC, I2C  # Import Pin, ADC, and I2C classes for hardware control
import time  # Import time module for delays and timing operations

# ========== BH1750 Light Sensor Driver Class ==========
PWR_DOWN = 0x00  # Power-down mode constant
PWR_ON = 0x01    # Power-on mode constant
RESET = 0x07     # Reset command constant
CONT_HIRES_MODE = 0x10  # Continuous high-resolution measurement mode

class BH1750:
    def __init__(self, i2c, addr=0x5c):
        self.i2c = i2c          # Store I2C bus instance
        self.addr = addr        # Store device I2C address (default: 0x5c)
        self.mode = CONT_HIRES_MODE  # Default measurement mode
        self.i2c.writeto(self.addr, bytearray([PWR_ON]))  # Power on the sensor
        self.i2c.writeto(self.addr, bytearray([RESET]))   # Reset sensor settings
        self.set_mode(self.mode)  # Apply default measurement mode

    def set_mode(self, mode):
        self.mode = mode                # Update measurement mode
        self.i2c.writeto(self.addr, bytearray([self.mode]))  # Send mode command to sensor

    def luminance(self):
        time.sleep_ms(180)  # Wait for measurement completion (required for BH1750)
        data = self.i2c.readfrom(self.addr, 2) # Read 2-byte data (MSB and LSB)
        return (data[0]<<8 | data[1]) / 1.2  # Convert to lux and return

# ========== Pin Configuration ==========
I2C_SDA = 2         # SDA pin for BH1750 (GPIO2)
I2C_SCL = 3         # SCL pin for BH1750 (GPIO3)
SOUND_PIN = 29      # ADC pin for sound sensor (GPIO29)
LED_PIN = 18        # Output pin for LED (GPIO18)

# ========== Initialization ==========
i2c = I2C(1, scl=Pin(I2C_SCL), sda=Pin(I2C_SDA))  # Initialize I2C bus 1 with specified pins
light_sensor = BH1750(i2c)  # Create BH1750 light sensor instance
sound_sensor = ADC(Pin(SOUND_PIN))  # Initialize sound sensor ADC
led = Pin(LED_PIN, Pin.OUT)  # Initialize LED as digital output

# ========== Configuration Parameters ==========
LUX_THRESHOLD = 100       # Light intensity threshold (lux) - turn LED off if above
SOUND_THRESHOLD = 2000    # Sound trigger threshold (ADC value)
LED_ON_TIME = 10          # LED stay-on duration (seconds) after last sound detection

# ========== State Variables ==========
led_on = False            # Track LED state (on/off)
last_sound_time = 0       # Timestamp of last sound detection

print("System started. Monitoring light and sound...")  # Startup message

# ========== Main Loop ==========
try:
    while True:
        lux = light_sensor.luminance()  # Read light intensity (lux)
        sound_value = sound_sensor.read_u16()  # Read raw sound sensor value (0-65535)
        now = time.time()  # Get current time (seconds since epoch)

        # Print debug information
        print("Light: {:.2f} lx, Sound: {}".format(lux, sound_value))

        if lux >= LUX_THRESHOLD:
            # High light intensity: force LED off
            led.value(0)
            led_on = False
        else:
            # Low light intensity: check sound trigger
            if sound_value >= SOUND_THRESHOLD:
                led.value(1)        # Turn LED on
                led_on = True       # Update LED state
                last_sound_time = now  # Record sound detection time
            else:
                # No sound detected: check if LED should turn off after delay
                if led_on:
                    if now - last_sound_time >= LED_ON_TIME:
                        led.value(0)    # Turn LED off after timeout
                        led_on = False  # Update LED state

        time.sleep(1)  # 1-second main loop interval

except KeyboardInterrupt:
    led.value(0)  # Ensure LED is off on keyboard interrupt
    print("Program terminated. LED turned off.")  # Shutdown message