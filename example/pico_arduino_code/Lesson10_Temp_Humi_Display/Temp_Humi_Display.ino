#include <LovyanGFX.hpp>    // Include the LovyanGFX library for display control
#include <Wire.h>           // Include the Wire library for I2C communication
#include "DHT20.h"          // Include the DHT20 library for temperature and humidity sensor

// Custom display class definition
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

LGFX gfx;           // Create an instance of the custom display class
DHT20 DHT(&Wire1);  // Create an instance of the DHT20 sensor using Wire1

unsigned long preMillis = 0; // Variable to store the last time the data was updated
const long interval = 1000; // Interval to update the data (1 second)

// Screen resolution definitions
static const uint16_t screenWidth = 320; // Define the screen width
static const uint16_t screenHeight = 240; // Define the screen height

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate

  gfx.init();                // Initialize the display
  pinMode(0, OUTPUT);        // Set pin 0 as an output for the backlight
  digitalWrite(0, HIGH);     // Turn on the backlight

  gfx.setTextColor(TFT_WHITE); // Set the text color to white
  gfx.setTextSize(3); // Set the text size to 3
  gfx.fillScreen(TFT_BLACK); // Clear the screen with black

  // Initialize Wire1 bus (for DHT20)
  Wire1.setSDA(2); // Set the SDA pin for Wire1
  Wire1.setSCL(3); // Set the SCL pin for Wire1
  Wire1.begin(); // Start the Wire1 bus

  DHT.begin();               // Initialize the DHT20 sensor
}

void loop() {
  unsigned long currentMillis = millis(); // Get the current time in milliseconds

  // Update data every second
  if (currentMillis - preMillis >= interval) { // Check if the interval has passed
    if (millis() - DHT.lastRead() >= 1000) { // Check if 1 second has passed since the last read
        int status = DHT.read(); // Read data from the DHT20 sensor
        if (status == DHT20_OK) { // Check if the read was successful
            gfx.fillScreen(TFT_BLACK);  // Clear the screen to prevent ghosting
            gfx.setTextSize(3); // Set the text size to 3

            // Get the values
            float humidity = DHT.getHumidity(); // Get the humidity value
            float temperature = DHT.getTemperature(); // Get the temperature value

            // Center calculation: font width is approximately 6 * textSize
            int centerX = screenWidth / 2; // Calculate the center X position

            // --- Display humidity ---
            gfx.setCursor(centerX - 6 * 3 * 5, 60);  // Set cursor position for the label (left-aligned center, estimated length of 5 characters)
            gfx.setTextColor(TFT_WHITE); // Set the text color to white
            gfx.print("Humidity:"); // Print the label

            gfx.setTextColor(TFT_BLUE); // Set the text color to blue
            gfx.setCursor(centerX - 6 * 3 * 2, 100);  // Set cursor position for the value (centered)
            gfx.printf("%.1f %%", humidity); // Print the humidity value with one decimal place

            // --- Display temperature ---
            gfx.setTextColor(TFT_WHITE); // Set the text color to white
            gfx.setCursor(centerX - 6 * 3 * 6, 160);  // Set cursor position for the label (estimated length of 10 characters)
            gfx.print("Temperature:"); // Print the label

            gfx.setTextColor(TFT_RED); // Set the text color to red
            gfx.setCursor(centerX - 6 * 3 * 2, 200);  // Set cursor position for the value (centered)
            gfx.printf("%.1f C", temperature); // Print the temperature value with one decimal place
        }
    }
    preMillis = currentMillis; // Update the last update time
  }
}