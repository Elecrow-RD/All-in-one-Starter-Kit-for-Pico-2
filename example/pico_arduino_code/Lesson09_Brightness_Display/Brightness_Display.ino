#include <LovyanGFX.hpp> // Include the LovyanGFX library for display control
#include <Arduino.h> // Include the Arduino core library

// === LovyanGFX Custom Display Class ===
class LGFX : public lgfx::LGFX_Device { // Custom display class inheriting from LGFX_Device
  lgfx::Panel_ST7789 _panel_instance; // Instance of the ST7789 panel driver
  lgfx::Bus_SPI _bus_instance; // Instance of the SPI bus
  lgfx::Touch_FT5x06 _touch_instance; // Instance of the FT5x06 touch driver

public:
  LGFX(void) { // Constructor for the LGFX class
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

// === Pin Definitions ===
#define SLIDER_PIN     28 // Define the pin connected to the analog slider
#define RED_LED_PIN    18 // Define the pin connected to the red LED
#define YELLOW_LED_PIN 20 // Define the pin connected to the yellow LED
#define GREEN_LED_PIN  19 // Define the pin connected to the green LED

int lastLevel = -1;  // Store the last brightness level

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate

  // LED setup
  pinMode(RED_LED_PIN, OUTPUT); // Set the red LED pin as an output
  pinMode(YELLOW_LED_PIN, OUTPUT); // Set the yellow LED pin as an output
  pinMode(GREEN_LED_PIN, OUTPUT); // Set the green LED pin as an output

  // Display setup
  gfx.init(); // Initialize the display
  pinMode(0, OUTPUT); // Set pin 0 as an output for the backlight
  digitalWrite(0, HIGH); // Turn on the backlight

  gfx.fillScreen(TFT_BLACK); // Clear the screen with black
  gfx.setTextSize(3); // Set the text size to 3
  gfx.setTextColor(TFT_WHITE); // Set the text color to white
  gfx.setCursor(30, 100); // Set the cursor position for the static text
  gfx.print("Brightness:"); // Print the static text
}

void loop() {
  int analogValue = analogRead(SLIDER_PIN); // Read the analog value from the slider (range is typically 0 to 4095)
  int pwmValue = map(analogValue, 0, 900, 0, 255); // Map the analog value from [0–900] to the PWM output range [0–255]
  pwmValue = constrain(pwmValue, 0, 255); // Constrain the PWM value to the range 0 to 255 to prevent out-of-range values

  int level = map(analogValue, 0, 900, 0, 10); // Map the analog value to an integer level value in the range [0–10]
  level = constrain(level, 0, 10); // Ensure the level is within the range 0 to 10

  // LED PWM output
  analogWrite(RED_LED_PIN, pwmValue); // Set the PWM value for the red LED
  analogWrite(YELLOW_LED_PIN, pwmValue); // Set the PWM value for the yellow LED
  analogWrite(GREEN_LED_PIN, pwmValue); // Set the PWM value for the green LED

  // Update the display if the brightness level has changed
  if (level != lastLevel) {
    lastLevel = level; // Update the last level

    // Clear the old level number area
    gfx.fillRect(240, 100, 60, 30, TFT_BLACK);
    gfx.setCursor(240, 100); // Set the cursor position for the level number
    gfx.setTextColor(TFT_GREEN); // Set the text color to green
    gfx.print(level); // Print the new level number

    // Print debug information to the serial monitor
    Serial.print("ADC: ");
    Serial.print(analogValue);
    Serial.print(" | PWM: ");
    Serial.print(pwmValue);
    Serial.print(" | Level: ");
    Serial.println(level);
  }

  delay(100); // Short delay to reduce CPU usage
}