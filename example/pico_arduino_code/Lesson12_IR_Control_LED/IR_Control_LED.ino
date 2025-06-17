#include <IRremote.h> // Include the IRremote library to handle infrared signals

const int IR_RECEIVE_PIN = 11;  // Define the pin connected to the infrared receiver

// LED pins
const int RED_LED_PIN = 18; // Define the pin connected to the red LED
const int YELLOW_LED_PIN = 20; // Define the pin connected to the yellow LED
const int GREEN_LED_PIN = 19; // Define the pin connected to the green LED

unsigned long lastEffectTime = 0; // Variable to store the last time an effect was updated
int effectMode = 0; // Variable to store the current effect mode (0=none, 1=blink, 2=running light)
int effectStep = 0; // Variable to store the current step in the running light effect

void setup() {
  Serial.begin(9600); // Initialize serial communication at 9600 baud rate
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Initialize the infrared receiver on the specified pin

  pinMode(RED_LED_PIN, OUTPUT); // Set the red LED pin as an output
  pinMode(YELLOW_LED_PIN, OUTPUT); // Set the yellow LED pin as an output
  pinMode(GREEN_LED_PIN, OUTPUT); // Set the green LED pin as an output

  turnOffAll(); // Turn off all LEDs initially
}

void loop() {
  // Handle infrared signals
  if (IrReceiver.decode()) { // Check if an infrared signal has been received
    unsigned long code = IrReceiver.decodedIRData.command; // Get the received IR code
    Serial.print("Received code: 0x"); // Print the received code in hexadecimal format
    Serial.println(code, HEX);

    handleIR(code); // Process the received IR code
    IrReceiver.resume(); // Resume listening for IR signals
  }

  // Handle lighting effects
  handleEffects(); // Update the lighting effects based on the current mode
}

void handleIR(unsigned long code) {
  turnOffAll(); // Turn off all LEDs before processing the new code
  effectMode = 0; // Reset the effect mode

  switch (code) {
    case 0x0C: // Code for button 1
      digitalWrite(RED_LED_PIN, HIGH); // Turn on the red LED
      Serial.println("Red light on"); // Print a message to the serial monitor
      break;
    case 0x18: // Code for button 2
      digitalWrite(YELLOW_LED_PIN, HIGH); // Turn on the yellow LED
      Serial.println("Yellow light on"); // Print a message to the serial monitor
      break;
    case 0x5E: // Code for button 3
      digitalWrite(GREEN_LED_PIN, HIGH); // Turn on the green LED
      Serial.println("Green light on"); // Print a message to the serial monitor
      break;
    case 0x08: // Code for button 4
      effectMode = 1; // Set the effect mode to blink
      Serial.println("Start blinking"); // Print a message to the serial monitor
      break;
    case 0x1C: // Code for button 5
      effectMode = 2; // Set the effect mode to running light
      Serial.println("Start running light"); // Print a message to the serial monitor
      break;
    case 0x5A: // Code for button 6
      Serial.println("All lights off"); // Print a message to the serial monitor
      break;
  }
}

void handleEffects() {
  if (effectMode == 1) { // Check if the effect mode is blink
    // All lights blink
    if (millis() - lastEffectTime > 300) { // Check if 300 milliseconds have passed since the last update
      static bool on = false; // Static variable to toggle the state
      on = !on; // Toggle the state
      digitalWrite(RED_LED_PIN, on); // Update the red LED state
      digitalWrite(YELLOW_LED_PIN, on); // Update the yellow LED state
      digitalWrite(GREEN_LED_PIN, on); // Update the green LED state
      lastEffectTime = millis(); // Update the last effect time
    }
  } else if (effectMode == 2) { // Check if the effect mode is running light
    // Running light effect
    if (millis() - lastEffectTime > 300) { // Check if 300 milliseconds have passed since the last update
      switch (effectStep % 3) { // Cycle through the three LEDs
        case 0:
          digitalWrite(RED_LED_PIN, HIGH); // Turn on the red LED
          digitalWrite(YELLOW_LED_PIN, LOW); // Turn off the yellow LED
          digitalWrite(GREEN_LED_PIN, LOW); // Turn off the green LED
          break;
        case 1:
          digitalWrite(RED_LED_PIN, LOW); // Turn off the red LED
          digitalWrite(YELLOW_LED_PIN, HIGH); // Turn on the yellow LED
          digitalWrite(GREEN_LED_PIN, LOW); // Turn off the green LED
          break;
        case 2:
          digitalWrite(RED_LED_PIN, LOW); // Turn off the red LED
          digitalWrite(YELLOW_LED_PIN, LOW); // Turn off the yellow LED
          digitalWrite(GREEN_LED_PIN, HIGH); // Turn on the green LED
          break;
      }
      effectStep++; // Increment the effect step
      lastEffectTime = millis(); // Update the last effect time
    }
  }
}

void turnOffAll() {
  digitalWrite(RED_LED_PIN, LOW); // Turn off the red LED
  digitalWrite(YELLOW_LED_PIN, LOW); // Turn off the yellow LED
  digitalWrite(GREEN_LED_PIN, LOW); // Turn off the green LED
}