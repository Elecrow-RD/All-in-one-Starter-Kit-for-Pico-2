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

# === === === === === === === === === === === === === === === === === === === === === === === ===

import ui                  # Import custom UI module for display control
import time                # Import time module for timing functions
import machine             # Import machine module for hardware interaction

# === Pin definitions ===
TRIG = machine.Pin(9, machine.Pin.OUT)   # Ultrasonic trigger pin (output)
ECHO = machine.Pin(8, machine.Pin.IN)    # Ultrasonic echo pin (input)

# === Get a valid distance measurement (unit: cm) ===
def measure_distance():
    TRIG.off()                         # Set trigger pin low
    time.sleep_us(2)                   # Wait 2 microseconds
    TRIG.on()                          # Set trigger pin high
    time.sleep_us(10)                  # Hold high for 10 microseconds
    TRIG.off()                         # Set trigger pin low again

    try:
        # Measure pulse duration (max 30ms timeout)
        pulse_time = machine.time_pulse_us(ECHO, 1, 30000)  
        # Calculate distance: sound speed 343m/s, convert to cm
        distance_cm = (pulse_time / 2) * 0.0343  
        return distance_cm             # Return valid distance
    except OSError:
        return -1                      # Return error on timeout/invalid data

# === Take 3 measurements and return average for stability ===
def get_stable_distance():
    total = 0                          # Initialize total distance
    count = 0                          # Initialize valid measurement count
    for _ in range(3):                 # Loop 3 times
        d = measure_distance()         # Get single distance measurement
        if 0 < d < 500:                # Check if distance is within valid range
            total += d                 # Accumulate valid distance
            count += 1                 # Increment valid count
        time.sleep_ms(20)              # Wait 20ms between measurements
    if count == 0:                     # If no valid measurements
        return -1                      # Return error
    return total / count               # Return average distance

# Initialize UI display
ui.ui_Label1.set_text("Distance: ...")  # Set initial display text

# Update interval configuration
UPDATE_INTERVAL = 500                  # Update display every 500ms
last_update = time.ticks_ms()          # Record last update time

# Main loop
while True:
    now = time.ticks_ms()              # Get current time
    # Check if update interval has elapsed
    if time.ticks_diff(now, last_update) >= UPDATE_INTERVAL:  
        last_update = now              # Update last update time
        distance = get_stable_distance()  # Get stable distance measurement

        if distance > 0:               # If valid distance received
            # Update UI with formatted distance
            ui.ui_Label1.set_text("Distance: {:.2f} cm".format(distance))  
        else:
            ui.ui_Label1.set_text("Out of range")  # Update UI for invalid distance

    time.sleep_ms(10)                  # Sleep 10ms to reduce CPU load
