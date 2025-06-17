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

from machine import I2C, Pin  # Import I2C and Pin classes from the machine module
from time import ticks_ms, ticks_diff, sleep_ms  # Import timing functions from the time module
from acceleration import LSM6DS3TRC  # Import the LSM6DS3TRC class for the accelerometer
import ui  # Import the ui module for updating the user interface

# Initialize I2C and the sensor
i2c = I2C(1, scl=Pin(3), sda=Pin(2))  # Initialize I2C on pins 3 (SCL) and 2 (SDA)
sensor = LSM6DS3TRC(i2c)  # Initialize the LSM6DS3TRC sensor using the I2C bus

# Initialize label objects
x_label = ui.ui_XLabel  # X-axis label object
y_label = ui.ui_YLabel  # Y-axis label object
z_label = ui.ui_ZLabel  # Z-axis label object

# Function to update the acceleration data and write it to the labels
def update_display():
    x, y, z = sensor.read_acceleration()  # Read the acceleration data from the sensor
    x_label.set_text("X: {:.2f} m/s^2".format(x))  # Update the X-axis label with the acceleration value
    y_label.set_text("Y: {:.2f} m/s^2".format(y))  # Update the Y-axis label with the acceleration value
    z_label.set_text("Z: {:.2f} m/s^2".format(z))  # Update the Z-axis label with the acceleration value

# Main loop
last_update = 0  # Variable to store the last update time
UPDATE_INTERVAL = 150  # Update interval in milliseconds

while True:  # Infinite loop to continuously update the display
    now = ticks_ms()  # Get the current time in milliseconds
    if ticks_diff(now, last_update) >= UPDATE_INTERVAL:  # Check if the update interval has passed
        update_display()  # Update the display with the latest acceleration data
        last_update = now  # Update the last update time
    lv.task_handler()  # Handle UI tasks to keep the interface responsive
    sleep_ms(5)  # Small delay to avoid excessive CPU usage
