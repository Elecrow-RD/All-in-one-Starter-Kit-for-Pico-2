#include <Servo.h> // Include the Servo library to control a servo motor
#include <IRremote.h> // Include the IRremote library to handle infrared signals
#include <LovyanGFX.hpp> // Include the LovyanGFX library for display control

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

const int IR_RECEIVE_PIN = 11;  // Define the pin connected to the infrared receiver

LGFX gfx;           // Create an instance of the custom display class
Servo myservo;      // Create a servo object

// ------------------ Control Variables ------------------
int angleInput = 0;      // Current input angle
bool servoEnabled = true; // Servo enable/disable flag

// ------------------ Function Definitions ------------------
void showAngleInput() {
  gfx.fillScreen(TFT_BLACK); // Clear the screen with black
  gfx.setTextSize(3); // Set the text size to 3
  gfx.setCursor(30, 100); // Set the cursor position
  gfx.setTextColor(TFT_WHITE); // Set the text color to white
  gfx.printf("Input: %d", angleInput); // Display the current input angle
}

void showRotatedAngle(int angle) {
  gfx.fillScreen(TFT_BLACK); // Clear the screen with black
  gfx.setCursor(30, 100); // Set the cursor position
  gfx.setTextSize(3); // Set the text size to 3
  gfx.setTextColor(TFT_GREEN); // Set the text color to green
  gfx.printf("Rotated to: %d", angle); // Display the angle the servo has rotated to
}

// ------------------ Initialization ------------------
void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  myservo.attach(13, 450, 2520); // Attach the servo to pin 13 with pulse width range
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Initialize the infrared receiver
  gfx.init(); // Initialize the display
  pinMode(0, OUTPUT); // Set pin 0 as an output for the backlight
  digitalWrite(0, HIGH); // Turn on the backlight
  showAngleInput(); // Display the initial angle input
}

// ------------------ Main Loop ------------------
void loop() {
  if (IrReceiver.decode()) { // Check if an infrared signal has been received
    unsigned long code = IrReceiver.decodedIRData.command; // Get the received IR code

    // ====== Debounce to improve button stability ======
    static uint32_t lastKeyCode = 0; // Store the last received code
    static unsigned long lastKeyTime = 0; // Store the last time a key was pressed
    const unsigned long debounceDelay = 300; // Debounce delay of 300ms
    if (code == lastKeyCode && (millis() - lastKeyTime) < debounceDelay) {
      IrReceiver.resume(); // Resume listening for IR signals
      return; // Exit the function if the same code is received within the debounce time
    }
    lastKeyCode = code; // Update the last received code
    lastKeyTime = millis(); // Update the last key press time
    // =========================

    switch (code) {
      case 0x45:  // [CH-] Disable the servo
        servoEnabled = false;
        Serial.println("Servo disabled");
        break;

      case 0x47:  // [CH+] Enable the servo
        servoEnabled = true;
        Serial.println("Servo enabled");
        break;

      case 0x43:  // [PLAY/PAUSE] Confirm input and rotate
        angleInput = constrain(angleInput, 0, 180); // Ensure the angle is within the valid range
        if (servoEnabled) {
          myservo.write(angleInput); // Rotate the servo to the input angle
          showRotatedAngle(angleInput); // Display the rotated angle
        } else {
          Serial.println("Servo is disabled. Cannot rotate.");
        }
        angleInput = 0; // Clear the input angle
        delay(1000); // Wait for 1 second
        break;

      // Handle numeric input 0-9
      case 0x16: angleInput = angleInput * 10 + 0; break;  // If IR code is 0x16, append digit 0 to angleInput
      case 0x0C: angleInput = angleInput * 10 + 1; break;  // If IR code is 0x0C, append digit 1 to angleInput
      case 0x18: angleInput = angleInput * 10 + 2; break;  // If IR code is 0x18, append digit 2 to angleInput
      case 0x5E: angleInput = angleInput * 10 + 3; break;  // If IR code is 0x5E, append digit 3 to angleInput
      case 0x08: angleInput = angleInput * 10 + 4; break;  // If IR code is 0x08, append digit 4 to angleInput
      case 0x1C: angleInput = angleInput * 10 + 5; break;  // If IR code is 0x1C, append digit 5 to angleInput
      case 0x5A: angleInput = angleInput * 10 + 6; break;  // If IR code is 0x5A, append digit 6 to angleInput
      case 0x42: angleInput = angleInput * 10 + 7; break;  // If IR code is 0x42, append digit 7 to angleInput
      case 0x52: angleInput = angleInput * 10 + 8; break;  // If IR code is 0x52, append digit 8 to angleInput
      case 0x4A: angleInput = angleInput * 10 + 9; break;  // If IR code is 0x4A, append digit 9 to angleInput


      default:
        Serial.print("Unknown code: ");
        Serial.println(code, HEX); // Print unknown codes to the serial monitor
        break;
    }

    // Limit input to a maximum of 3 digits
    if (angleInput > 999) angleInput = 0;
    showAngleInput(); // Update the displayed angle input

    IrReceiver.resume(); // Resume listening for IR signals
  }
}