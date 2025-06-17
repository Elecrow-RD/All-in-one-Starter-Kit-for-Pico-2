#include <LovyanGFX.hpp> // Include the LovyanGFX library for display control

// LED pin definitions
#define Red_LED 18 // Define the pin for the red LED
#define Yellow_LED 20 // Define the pin for the yellow LED
#define Green_LED 19 // Define the pin for the green LED

// Custom LGFX device class
// Screen driver code for pico2
class LGFX : public lgfx::LGFX_Device // Inherit from LGFX_Device
{
  lgfx::Panel_ST7789 _panel_instance; // Instance of the ST7789 panel driver
  lgfx::Bus_SPI _bus_instance; // Instance of the SPI bus
  lgfx::Touch_FT5x06 _touch_instance; // Instance of the FT5x06 touch driver

public:
  LGFX(void)
  {
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

LGFX gfx; // Create an instance of the screen
static const uint16_t screenWidth = 320; // Define the screen width
static const uint16_t screenHeight = 240; // Define the screen height

// Enum for the stages of the traffic light
enum Stage { GREEN_STAGE, YELLOW_STAGE, RED_STAGE };

// Global variables for the current stage and remaining time
Stage currentStage = GREEN_STAGE; // Current stage of the traffic light
uint32_t remainingTime = 30; // Remaining time for the current stage
uint32_t lastUpdate = 0; // Last time the countdown was updated
uint32_t lastBlink = 0; // Last time the LED blinked
bool blinkState = false; // State of the blinking LED

// Constants for the countdown text area dimensions
const int TEXT_AREA_X = 210; // X coordinate of the text area
const int TEXT_AREA_Y = 100; // Y coordinate of the text area
const int TEXT_AREA_WIDTH = 60; // Width of the text area
const int TEXT_AREA_HEIGHT = 30; // Height of the text area

void setup() {
  Serial.begin(115200); // Initialize the serial port for debugging
  
  // Initialize the screen and backlight
  gfx.init(); // Initialize the screen
  pinMode(0, OUTPUT); // Set pin 0 as an output for the backlight
  digitalWrite(0, HIGH); // Turn on the backlight
  
  // Initialize the LED pins
  pinMode(Red_LED, OUTPUT); // Set the red LED pin as an output
  pinMode(Yellow_LED, OUTPUT); // Set the yellow LED pin as an output
  pinMode(Green_LED, OUTPUT); // Set the green LED pin as an output
  
  // Initial state: Green light on, display countdown in English
  digitalWrite(Green_LED, HIGH); // Turn on the green LED
  gfx.setTextSize(3); // Set the text size to 3
  gfx.setTextColor(TFT_WHITE); // Set the text color to white
  
  // Display the static text part (unchanging)
  gfx.setCursor(30, 100); // Set the cursor position for the static text
  gfx.print("Countdown:"); // Print the static text
  
  // Display the initial number
  gfx.setCursor(TEXT_AREA_X, TEXT_AREA_Y); // Set the cursor position for the countdown
  gfx.printf("%2ds", remainingTime); // Print the initial countdown value
  
  lastUpdate = millis(); // Record the initial time
}

void loop() {
  uint32_t now = millis(); // Get the current time in milliseconds

  // Update the countdown logic every second
  if (now - lastUpdate >= 1000) {
    remainingTime--; // Decrement the remaining time
    lastUpdate = now; // Update the last update time

    // Clear only the number area (avoid clearing the "Countdown:" text)
    gfx.fillRect(TEXT_AREA_X, TEXT_AREA_Y, TEXT_AREA_WIDTH, TEXT_AREA_HEIGHT, TFT_BLACK);
    
    // Redisplay the new number
    gfx.setTextColor(TFT_WHITE); // Set the text color to white
    gfx.setCursor(TEXT_AREA_X, TEXT_AREA_Y); // Set the cursor position for the countdown
    gfx.printf("%2ds", remainingTime); // Print the updated countdown value

    // Time's up, switch stage
    if (remainingTime == 0) {
      // Turn off all LEDs
      digitalWrite(Green_LED, LOW); // Turn off the green LED
      digitalWrite(Yellow_LED, LOW); // Turn off the yellow LED
      digitalWrite(Red_LED, LOW); // Turn off the red LED

      // Switch stage and initialize new time
      switch(currentStage) {
        case GREEN_STAGE:
          currentStage = YELLOW_STAGE; // Switch to yellow stage
          remainingTime = 3; // Set remaining time for yellow stage
          digitalWrite(Yellow_LED, HIGH); // Turn on the yellow LED
          break;
          
        case YELLOW_STAGE:
          currentStage = RED_STAGE; // Switch to red stage
          remainingTime = 20; // Set remaining time for red stage
          digitalWrite(Red_LED, HIGH); // Turn on the red LED
          break;
          
        case RED_STAGE:
          currentStage = GREEN_STAGE; // Switch to green stage
          remainingTime = 30; // Set remaining time for green stage
          digitalWrite(Green_LED, HIGH); // Turn on the green LED
          break;
      }
    }
  }

  // Handle the last 5 seconds blink logic (keep the original code unchanged)
  if (remainingTime > 0 && remainingTime <= 5) {
    if (now - lastBlink >= 500) { // Check if half a second has passed
      blinkState = !blinkState; // Toggle the blink state
      lastBlink = now; // Update the last blink time
      
      switch(currentStage) {
        case GREEN_STAGE:
          digitalWrite(Green_LED, blinkState); // Blink the green LED
          break;
        case RED_STAGE:
          digitalWrite(Red_LED, blinkState); // Blink the red LED
          break;
      }
    }
  } else {
    // Non-blinking stage, keep the LED on (keep the original code unchanged)
    switch(currentStage) {
      case GREEN_STAGE:
        digitalWrite(Green_LED, HIGH); // Turn on the green LED
        break;
      case YELLOW_STAGE:
        digitalWrite(Yellow_LED, HIGH); // Turn on the yellow LED
        break;
      case RED_STAGE:
        digitalWrite(Red_LED, HIGH); // Turn on the red LED
        break;
    }
  }

  delay(10); // Keep the minimum delay for LVGL
}