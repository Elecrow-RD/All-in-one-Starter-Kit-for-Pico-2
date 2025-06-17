// Include necessary libraries
#include <LovyanGFX.hpp> // Include the LovyanGFX library for display control
#include <IRremote.hpp> // Include the IRremote library for infrared signal handling
#include <stack> // Include the standard library for using std::stack

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
String input = ""; // Variable to store the input expression
bool evaluated = false; // Flag to indicate if the expression has been evaluated

// Function to display the input on the screen
void showInput(const String &text) {
  gfx.fillScreen(TFT_BLACK); // Clear the screen with black
  gfx.setCursor(10, 100); // Set the cursor position
  gfx.setTextSize(2); // Set the text size
  gfx.setTextColor(TFT_WHITE); // Set the text color to white
  gfx.print("Input: " + text); // Display the input text
  gfx.flush(); // Update the display
}

// Function to display the result on the screen
void showResult(const String &text) {
  gfx.fillScreen(TFT_BLACK); // Clear the screen with black
  gfx.setCursor(10, 100); // Set the cursor position
  gfx.setTextSize(2); // Set the text size
  gfx.setTextColor(TFT_GREEN); // Set the text color to green
  gfx.print("Result: " + text); // Display the result text
  gfx.flush(); // Update the display
}

// Function to determine the precedence of an operator
int precedence(char op) {
  if (op == '+' || op == '-') return 1; // Lower precedence for + and -
  if (op == '*' || op == '/') return 2; // Higher precedence for * and /
  return 0; // Default precedence
}

// Function to check if a character is an operator
bool isOperator(char c) {
  return c == '+' || c == '-' || c == '*' || c == '/'; // Check if the character is an operator
}

// Function to convert an infix expression to postfix notation
bool infixToPostfix(const String &infix, String &postfix) {
  std::stack<char> stack; // Stack to hold operators
  String num = ""; // Temporary string to hold numbers

  for (int i = 0; i < infix.length(); ++i) {
    char c = infix[i];

    if (c == ' ') continue; // Skip spaces

    if (c == '-' && (i == 0 || isOperator(infix[i - 1]))) {
      num += c; // Handle unary minus
      continue;
    }

    if (isdigit(c) || c == '.') {
      num += c; // Append digits and decimal points to the number
    } else if (isOperator(c)) {
      if (num.length() > 0) {
        postfix += num + " "; // Append the number to the postfix expression
        num = "";
      }
      while (!stack.empty() && precedence(stack.top()) >= precedence(c)) {
        postfix += stack.top(); // Pop operators from the stack and append to postfix
        postfix += " ";
        stack.pop();
      }
      stack.push(c); // Push the current operator onto the stack
    } else {
      return false; // Invalid character
    }
  }

  if (num.length() > 0) postfix += num + " "; // Append the last number
  while (!stack.empty()) {
    postfix += stack.top(); // Pop remaining operators from the stack
    postfix += " ";
    stack.pop();
  }

  return true; // Conversion successful
}

// Function to evaluate a postfix expression
float evaluatePostfix(const String &postfix) {
  std::stack<float> stack; // Stack to hold operands
  String token = ""; // Temporary string to hold tokens

  for (int i = 0; i <= postfix.length(); ++i) {  // Iterate through each character in the postfix expression
    char c = postfix[i];  // Get current character
    if (c == ' ' || c == '\0') {  // If space or null terminator is encountered
      if (token.length() > 0) {  // If a token has been accumulated
        if (token.length() == 1 && isOperator(token[0])) {  // If token is a single operator
          if (stack.size() < 2) return NAN; // Not enough operands
          float b = stack.top(); stack.pop();  // Pop second operand
          float a = stack.top(); stack.pop();  // Pop first operand
          switch (token[0]) {  // Perform operation based on operator
            case '+': stack.push(a + b); break;  // Addition
            case '-': stack.push(a - b); break;  // Subtraction
            case '*': stack.push(a * b); break;  // Multiplication
            case '/': if (b == 0) return NAN; stack.push(a / b); break; // Handle division by zero
          }
        } else {
          stack.push(token.toFloat()); // Convert token to float and push onto stack
        }
        token = ""; // Reset token
      }
    } else {
      token += c; // Build the token
    }
  }
  return (stack.size() == 1) ? stack.top() : NAN; // Return the result if stack size is 1, otherwise return NAN
}


// Function to evaluate an infix expression
float evaluateExpression(const String &expr) {
  String postfix;
  if (!infixToPostfix(expr, postfix)) return NAN; // Convert infix to postfix and check for errors
  return evaluatePostfix(postfix); // Evaluate the postfix expression
}

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  gfx.init(); // Initialize the display
  pinMode(0, OUTPUT); // Set pin 0 as an output for the backlight
  digitalWrite(0, HIGH); // Turn on the backlight
  showInput(""); // Display the initial input
  IrReceiver.begin(11, DISABLE_LED_FEEDBACK); // Initialize the IR receiver
}

void loop() {
  if (IrReceiver.decode()) { // Check if an IR signal has been received
    uint8_t code = IrReceiver.decodedIRData.command; // Get the received IR code
    Serial.print("Received: 0x");
    Serial.println(code, HEX); // Print the received code in hexadecimal

    static uint32_t lastKeyCode = 0; // Store the last received code
    static unsigned long lastKeyTime = 0; // Store the last key press time
    const unsigned long debounceDelay = 300; // Debounce delay in milliseconds
    if (code == lastKeyCode && (millis() - lastKeyTime) < debounceDelay) {
      IrReceiver.resume(); // Resume listening for IR signals
      return; // Exit if the same code was received within the debounce time
    }

    lastKeyCode = code; // Update the last received code
    lastKeyTime = millis(); // Update the last key press time

    if (evaluated) {
      input = ""; // Clear the input if the expression has been evaluated
      evaluated = false; // Reset the evaluated flag
    }

    switch (code) {
      case 0x16: input += "0"; break; // Append '0' to the input
      case 0x0C: input += "1"; break; // Append '1' to the input
      case 0x18: input += "2"; break; // Append '2' to the input
      case 0x5E: input += "3"; break; // Append '3' to the input
      case 0x08: input += "4"; break; // Append '4' to the input
      case 0x1C: input += "5"; break; // Append '5' to the input
      case 0x5A: input += "6"; break; // Append '6' to the input
      case 0x42: input += "7"; break; // Append '7' to the input
      case 0x52: input += "8"; break; // Append '8' to the input
      case 0x4A: input += "9"; break; // Append '9' to the input
      case 0x15: input += "+"; break; // Append '+' to the input
      case 0x07: input += "-"; break; // Append '-' to the input
      case 0x44: input += "/"; break; // Append '/' to the input
      case 0x40: input += "*"; break; // Append '*' to the input
      case 0x09: input = ""; break; // Clear the input
      case 0x43: {  // OK = Calculate
        float result = evaluateExpression(input); // Evaluate the input expression
        if (isnan(result)) {
          showResult("Error"); // Display error if the result is NaN
        } else if (!isfinite(result)) {
          showResult("Overflow"); // Display overflow if the result is not finite
        } else {
          showResult(String(result, 2));  // Display the result with two decimal places
        }
        evaluated = true; // Set the evaluated flag
        IrReceiver.resume(); // Resume listening for IR signals
        return;
      }
      case 0x19: { // Append decimal point '.'
        // Prevent consecutive decimal points or placing a decimal point directly after an operator
        if (input.length() == 0 || input.endsWith(".") || isOperator(input[input.length() - 1])) {
          break; // Do nothing if invalid
        }
        // Check if the current number already has a decimal point
        int i = input.length() - 1;
        while (i >= 0 && (isdigit(input[i]) || input[i] == '.')) {
          if (input[i] == '.') break; // Break if a decimal point is found
          --i;
        }
        if (i >= 0 && input[i] == '.') break;  // Do nothing if the current number already has a decimal point
        input += "."; // Append the decimal point
        break;
      }
      default:
        Serial.print("Unknown key: 0x");
        Serial.println(code, HEX); // Print unknown keys in hexadecimal
        break;
    }

    showInput(input); // Update the display with the current input
    IrReceiver.resume(); // Resume listening for IR signals
  }
}
