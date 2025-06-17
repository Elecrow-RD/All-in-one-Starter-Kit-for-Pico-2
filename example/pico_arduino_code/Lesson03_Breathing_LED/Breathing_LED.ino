#include <Arduino.h>

// === Pin Definitions ===
#define SLIDER_PIN     28   // Slider potentiometer connected to GP28 (ADC2)
#define RED_LED_PIN    18   // Red LED connected to GPIO18 (PWM capable)
#define YELLOW_LED_PIN 20   // Yellow LED connected to GPIO20 (PWM capable)
#define GREEN_LED_PIN  19   // Green LED connected to GPIO19 (PWM capable)

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  while (!Serial); // Wait for serial connection (optional)

  // Configure LED pins as outputs
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  // Indicate system initialization complete
  Serial.println("LED Brightness Control with Slider Ready.");
}

void loop() {
  // Read analog value from slider (RP2040 ADC is 12-bit, range 0-4095)
  int analogValue = analogRead(SLIDER_PIN);

  // Map ADC value (0-4095) to PWM range (0-255)
  int pwmValue = map(analogValue, 0, 4095, 0, 255);

  // Synchronously control LED brightness using PWM
  analogWrite(RED_LED_PIN, pwmValue);
  analogWrite(YELLOW_LED_PIN, pwmValue);
  analogWrite(GREEN_LED_PIN, pwmValue);

  // Print debug information
  Serial.print("ADC: ");
  Serial.print(analogValue);
  Serial.print(" | PWM: ");
  Serial.println(pwmValue);

  delay(100); // Control refresh rate to reduce serial output frequency
}