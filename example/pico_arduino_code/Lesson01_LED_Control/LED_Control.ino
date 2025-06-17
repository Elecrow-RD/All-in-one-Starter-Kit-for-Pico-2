// Include the Arduino core library
#include <Arduino.h>

// Define LED pins
#define Red_LED 18      // Red LED connected to digital pin 18
#define Yellow_LED 20   // Yellow LED connected to digital pin 20
#define Green_LED 19    // Green LED connected to digital pin 19

// Variable to record the last time of state change
unsigned long previousMillis = 0;
// LED blinking interval: 1000 milliseconds = 1 second
const long interval = 1000;

// Current states of the three LEDs (HIGH = on, LOW = off)
int redState = LOW;
int yellowState = LOW;
int greenState = LOW;

// Initialization function: Set pin modes
void setup() {
  // Configure LED pins as output
  pinMode(Red_LED, OUTPUT);
  pinMode(Yellow_LED, OUTPUT);
  pinMode(Green_LED, OUTPUT);

  // Set initial state (all LEDs off)
  digitalWrite(Red_LED, redState);
  digitalWrite(Yellow_LED, yellowState);
  digitalWrite(Green_LED, greenState);
}

// Main loop function: Control LED blinking
void loop() {
  // Get current running time in milliseconds
  unsigned long currentMillis = millis();

  // Check if the interval has elapsed
  if (currentMillis - previousMillis >= interval) {
    // Update the time record
    previousMillis = currentMillis;

    // Toggle LED states (on → off, off → on)
    redState = (redState == LOW) ? HIGH : LOW;
    yellowState = (yellowState == LOW) ? HIGH : LOW;
    greenState = (greenState == LOW) ? HIGH : LOW;

    // Apply new states to the pins
    digitalWrite(Red_LED, redState);
    digitalWrite(Yellow_LED, yellowState);
    digitalWrite(Green_LED, greenState);
  }
}