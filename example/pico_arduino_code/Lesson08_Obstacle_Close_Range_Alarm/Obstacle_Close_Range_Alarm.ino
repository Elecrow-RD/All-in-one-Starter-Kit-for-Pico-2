#include <HCSR04.h> // Include the HCSR04 library to use the ultrasonic distance sensor

// Pin definitions
const byte triggerPin = 9; // Define the pin connected to the trigger of the ultrasonic sensor
const byte echoPin = 8; // Define the pin connected to the echo of the ultrasonic sensor
const int vibratePin = 15; // Define the pin connected to the vibration motor

// Create an instance of the ultrasonic sensor object
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

// Update interval in milliseconds
const unsigned long UPDATE_INTERVAL = 100; // Set the interval between distance measurements
unsigned long lastUpdateTime = 0; // Variable to store the last time the distance was updated

void setup() {
  pinMode(vibratePin, OUTPUT);   // Set the vibration motor pin as an output
  digitalWrite(vibratePin, LOW); // Initially turn off the vibration motor

  Serial.begin(9600);            // Initialize serial communication at 9600 baud rate
  while (!Serial);               // Wait for serial connection (for some development boards)
  Serial.println("System ready. Starting distance monitoring..."); // Print a startup message indicating the system is ready
}

void loop() {
  unsigned long currentTime = millis(); // Get the current time in milliseconds
  if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) { // Check if the time elapsed since the last update is greater than or equal to the update interval
    lastUpdateTime = currentTime; // Update the last update time to the current time

    float distance = distanceSensor.measureDistanceCm(); // Measure the distance in centimeters using the ultrasonic sensor

    // Check the validity of the measurement
    if (distance > 0) { // If the measured distance is greater than 0 (valid measurement)
      Serial.print("Distance: "); // Print the measured distance
      Serial.print(distance);
      Serial.println(" cm");

      if (distance < 30.0) { // If the distance is less than 30 cm
        digitalWrite(vibratePin, HIGH); // Turn on the vibration motor
        Serial.println("Obstacle detected! Vibration motor ON."); // Print a message indicating an obstacle was detected
      } else {
        digitalWrite(vibratePin, LOW); // Turn off the vibration motor
        Serial.println("Safe distance. Vibration motor OFF."); // Print a message indicating a safe distance
      }

    } else {
      // Measurement is out of range or invalid
      digitalWrite(vibratePin, LOW); // Turn off the vibration motor for safety
      Serial.println("Out of range or measurement error."); // Print an error message
    }
  }

  delay(10); // Short delay to avoid high-frequency measurements and reduce CPU usage
}