#include <LovyanGFX.hpp> // Include the LovyanGFX library for display control
#include <Adafruit_LSM6DS3TRC.h> // Include the Adafruit LSM6DS3TRC library for the accelerometer sensor
#include <Wire.h> // Include the Wire library for I2C communication

// Custom LGFX device class
// Screen driver code for pico2
class LGFX : public lgfx::LGFX_Device { // Inherit from LGFX_Device
  lgfx::Panel_ST7789 _panel_instance; // Instance of the ST7789 panel driver
  lgfx::Bus_SPI _bus_instance; // Instance of the SPI bus
  lgfx::Touch_FT5x06 _touch_instance; // Instance of the FT5x06 touch driver

public:
  LGFX(void) {
    // SPI bus configuration
    auto cfg = _bus_instance.config(); // Get the default SPI bus configuration
    cfg.spi_host = 0; // Set the SPI host to 0
    cfg.spi_mode = 0; // Set the SPI mode to 0
    cfg.freq_write = 80000000; // Set the write frequency to 80 MHz
    cfg.freq_read = 16000000; // Set the read frequency to 16 MHz
    cfg.pin_sclk = 6; // Set the SCLK pin to 6
    cfg.pin_mosi = 7; // Set the MOSI pin to 7
    cfg.pin_miso = -1; // Set the MISO pin to -1 (not used)
    cfg.pin_dc = 16; // Set the DC pin to 16
    _bus_instance.config(cfg); // Apply the SPI bus configuration
    _panel_instance.setBus(&_bus_instance); // Assign the SPI bus to the panel

    // Screen panel configuration
    auto panel_cfg = _panel_instance.config(); // Get the default panel configuration
    panel_cfg.pin_cs = 17; // Set the CS pin to 17
    panel_cfg.pin_rst = -1; // Set the RST pin to -1 (not used)
    panel_cfg.pin_busy = -1; // Set the BUSY pin to -1 (not used)
    panel_cfg.memory_width = 240; // Set the memory width to 240
    panel_cfg.memory_height = 320; // Set the memory height to 320
    panel_cfg.panel_width = 240; // Set the panel width to 240
    panel_cfg.panel_height = 320; // Set the panel height to 320
    panel_cfg.offset_x = 0; // Set the X offset to 0
    panel_cfg.offset_y = 0; // Set the Y offset to 0
    panel_cfg.offset_rotation = 1; // Set the rotation offset to 1
    panel_cfg.dummy_read_pixel = 8; // Set the dummy read pixel to 8
    panel_cfg.dummy_read_bits = 1; // Set the dummy read bits to 1
    panel_cfg.readable = false; // Set the panel as not readable
    panel_cfg.invert = true; // Set the panel to invert colors
    panel_cfg.rgb_order = false; // Set the RGB order to false
    panel_cfg.dlen_16bit = false; // Set the data length to 16-bit
    panel_cfg.bus_shared = false; // Set the bus as not shared
    _panel_instance.config(panel_cfg); // Apply the panel configuration

    // Touch configuration
    auto touch_cfg = _touch_instance.config(); // Get the default touch configuration
    touch_cfg.x_min = 0; // Set the minimum X value to 0
    touch_cfg.x_max = 239; // Set the maximum X value to 239
    touch_cfg.y_min = 0; // Set the minimum Y value to 0
    touch_cfg.y_max = 319; // Set the maximum Y value to 319
    touch_cfg.pin_int = 25; // Set the INT pin to 25
    touch_cfg.pin_rst = 24; // Set the RST pin to 24
    touch_cfg.bus_shared = false; // Set the bus as not shared
    touch_cfg.offset_rotation = 0; // Set the rotation offset to 0
    touch_cfg.i2c_port = 0; // Set the I2C port to 0
    touch_cfg.i2c_addr = 0x38; // Set the I2C address to 0x38
    touch_cfg.pin_sda = 4; // Set the SDA pin to 4
    touch_cfg.pin_scl = 5; // Set the SCL pin to 5
    touch_cfg.freq = 400000; // Set the I2C frequency to 400 kHz
    _touch_instance.config(touch_cfg); // Apply the touch configuration
    _panel_instance.setTouch(&_touch_instance); // Assign the touch driver to the panel

    setPanel(&_panel_instance); // Set the panel instance for the device
  }
};

LGFX gfx; // Create an instance of the custom display class
Adafruit_LSM6DS3TRC lsm6ds3trc; // Create an instance of the LSM6DS3TRC accelerometer sensor

// Display parameter definitions
const uint16_t SCREEN_WIDTH = 240; // Actual effective display width (adjust according to screen rotation)
const uint16_t SCREEN_HEIGHT = 320; // Actual effective display height
const uint16_t UPDATE_INTERVAL = 150; // Data refresh interval (150ms)
const uint8_t TEXT_SIZE = 2; // Text size (affects character width and height)
const uint16_t TEXT_COLOR = TFT_WHITE; // Text color
const uint16_t BG_COLOR = TFT_BLACK; // Background color

// Axis display area coordinates (adjust according to screen layout)
const struct {
  uint16_t x_label; // Label X coordinate
  uint16_t y_label; // Label Y coordinate
  uint16_t x_value; // Value X coordinate
  uint16_t y_value; // Value Y coordinate
  uint16_t w; // Value area width (wide enough to cover the maximum possible value)
  uint16_t h; // Value area height
} AXIS_AREA[3] = {
  {20, 60, 80, 60, 150, 24}, // X-axis area (label at 20,60; value at 80,60; width 150 height 24)
  {20, 120, 80, 120, 150, 24}, // Y-axis area
  {20, 180, 80, 180, 150, 24} // Z-axis area
};

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate

  // Screen initialization
  gfx.init(); // Initialize the display
  pinMode(0, OUTPUT); // Set pin 0 as an output for the backlight
  digitalWrite(0, HIGH); // Turn on the backlight
  gfx.fillScreen(BG_COLOR); // Clear the screen with the background color

  // Sensor initialization
  Wire1.setSDA(2); // Configure the I2C SDA pin (adjust according to hardware connection)
  Wire1.setSCL(3); // Configure the I2C SCL pin
  Wire1.begin(); // Start the I2C bus
  delay(100); // Wait for 100 milliseconds

  if (!lsm6ds3trc.begin_I2C(0x6B, &Wire1)) { // Initialize the sensor with I2C address 0x6B
    gfx.setTextColor(TFT_RED); // Set text color to red
    gfx.setTextSize(TEXT_SIZE); // Set text size
    gfx.setCursor(20, SCREEN_HEIGHT / 2 - 20); // Set cursor position
    gfx.print("Sensor initialization failed!"); // Display error message
    while (1) delay(10); // Stop execution if initialization fails
  }

  // Draw fixed labels
  gfx.setTextSize(TEXT_SIZE); // Set text size
  gfx.setTextColor(TEXT_COLOR); // Set text color
  gfx.setCursor(20, 20); // Set cursor position for the title
  gfx.print("Accelerometer data"); // Display title
  gfx.setCursor(AXIS_AREA[0].x_label, AXIS_AREA[0].y_label); // Set cursor position for X-axis label
  gfx.print("X:"); // Display X-axis label
  gfx.setCursor(AXIS_AREA[1].x_label, AXIS_AREA[1].y_label); // Set cursor position for Y-axis label
  gfx.print("Y:"); // Display Y-axis label
  gfx.setCursor(AXIS_AREA[2].x_label, AXIS_AREA[2].y_label); // Set cursor position for Z-axis label
  gfx.print("Z:"); // Display Z-axis label
}

void loop() {
  static unsigned long lastUpdate = 0; // Variable to store the last update time
  if (millis() - lastUpdate < UPDATE_INTERVAL) return; // Check if the update interval has passed
  lastUpdate = millis(); // Update the last update time

  // Read accelerometer data
  sensors_event_t accel; // Create an event structure to store accelerometer data
  lsm6ds3trc.getEvent(&accel, nullptr, nullptr); // Get only the accelerometer data

  // Refresh display for each axis (X/Y/Z)
  for (int i = 0; i < 3; i++) {
    float value = 0; // Variable to store the value
    switch (i) {
      case 0: value = accel.acceleration.x; break; // Get X-axis value
      case 1: value = accel.acceleration.y; break; // Get Y-axis value
      case 2: value = accel.acceleration.z; break; // Get Z-axis value
    }

    // 1. Erase the old data area (key operation to eliminate ghosting)
    gfx.fillRect(
      AXIS_AREA[i].x_value, // X coordinate of the value area
      AXIS_AREA[i].y_value, // Y coordinate of the value area
      AXIS_AREA[i].w, // Width of the value area
      AXIS_AREA[i].h, // Height of the value area
      BG_COLOR // Background color
    );

    // 2. Draw the new value(Keep 2 decimal places)
    gfx.setTextColor(TEXT_COLOR); // Set text color
    gfx.setCursor(AXIS_AREA[i].x_value, AXIS_AREA[i].y_value); // Set cursor position
    gfx.printf("%.2f m/s^2", value); // Display the value with two decimal places
  }
}
