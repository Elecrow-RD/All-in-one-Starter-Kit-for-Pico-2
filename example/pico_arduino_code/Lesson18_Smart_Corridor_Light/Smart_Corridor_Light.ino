#include <Wire.h> // Include the Wire library for I2C communication
#include <BH1750.h> // Include the BH1750 library for the light sensor

// Light sensor object with I2C address 0x5c
BH1750 lightMeter(0x5c);

// Pin definitions
#define I2C_SDA 2 // Define the SDA pin for I2C communication
#define I2C_SCL 3 // Define the SCL pin for I2C communication
#define SOUND_PIN 29 // Define the pin connected to the sound sensor
#define LedPin 18 // Define the pin connected to the LED

void setup() {
    Serial.begin(9600); // Initialize serial communication at 9600 baud rate

    // I2C initialization
    Wire1.setSDA(I2C_SDA); // Set the SDA pin for I2C
    Wire1.setSCL(I2C_SCL); // Set the SCL pin for I2C
    Wire1.begin(); // Start the I2C bus

    if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5c, &Wire1)) {
        Serial.println(F("BH1750 begin success")); // Print success message if the sensor initializes correctly
    } else {
        Serial.println(F("BH1750 init failed")); // Print error message if the sensor fails to initialize
    }

    pinMode(LedPin, OUTPUT); // Set the LED pin as an output
    pinMode(SOUND_PIN, INPUT); // Set the sound sensor pin as an input
}

void loop() {
    static bool isSoundDetected = false; // Flag to indicate if sound has been detected
    static bool isLedOn = false; // Flag to indicate if the LED is currently on
    static unsigned long ledOnTimestamp = 0; // Timestamp for when the LED was turned on

    if (lightMeter.measurementReady(true)) { // Check if the light sensor has a new measurement
        int lux = lightMeter.readLightLevel(); // Read the light intensity in lux
        Serial.print("Light Intensity: "); // Print the light intensity
        Serial.print(lux);
        Serial.println(" lx");

        if (lux >= 100) {
            // Light is strong, force the LED to turn off
            Serial.println("[ACTION] Light strong - turning OFF LED");
            digitalWrite(LedPin, LOW); // Turn off the LED
            isLedOn = false; // Update the LED state flag
            isSoundDetected = false; // Reset the sound detection flag
            ledOnTimestamp = 0; // Reset the timestamp
        } else {
            // Light is dim, allow sound to control the LED
            if (digitalRead(SOUND_PIN)) { // Check if sound is detected
                Serial.println("[SOUND] Sound detected - turning ON LED");
                digitalWrite(LedPin, HIGH); // Turn on the LED
                isLedOn = true; // Update the LED state flag
                isSoundDetected = true; // Set the sound detection flag
                ledOnTimestamp = millis(); // Record the timestamp when the LED is turned on
            } else {
                if (isSoundDetected) {
                    // No sound detected, but sound was previously detected, keep the LED on for 10 seconds
                    if (millis() - ledOnTimestamp < 10000) {
                        Serial.println("[ACTION] Within 10s - keeping LED ON");
                        digitalWrite(LedPin, HIGH); // Keep the LED on
                        isLedOn = true; // Update the LED state flag
                    } else {
                        Serial.println("[ACTION] 10s passed - turning OFF LED");
                        digitalWrite(LedPin, LOW); // Turn off the LED
                        isLedOn = false; // Update the LED state flag
                        isSoundDetected = false; // Reset the sound detection flag
                    }
                } else {
                    Serial.println("[ACTION] No recent sound - turning OFF LED");
                    digitalWrite(LedPin, LOW); // Turn off the LED
                    isLedOn = false; // Update the LED state flag
                }
            }
        }
    }

    delay(10); // Short delay to reduce CPU usage
}
