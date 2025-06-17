#include <Wire.h> // Include the Wire library for I2C communication
#include <BH1750.h> // Include the BH1750 library for light sensor

// Define the I2C interface pins for SDA and SCL
#define I2C_SDA 2 // Define the SDA pin
#define I2C_SCL 3 // Define the SCL pin

// Define the red LED interface pin
#define Red_LED 18 // Define the pin for the red LED

// Instantiate a BH1750 object with the address 0x5C
BH1750 lightMeter(0x5c); // Create an instance of the BH1750 sensor

void setup() {
  // Initialize the serial port for outputting debug information
  Serial.begin(9600); // Start serial communication at 9600 baud rate
  delay(100); // Wait for the serial port to stabilize

  // Initialize the I2C port
  Wire1.setSDA(I2C_SDA); // Set the SDA pin for I2C
  Wire1.setSCL(I2C_SCL); // Set the SCL pin for I2C
  Wire1.begin(); // Start the I2C bus

  // Initialize the BH1750 sensor
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5c, &Wire1)) {
    Serial.println("BH1750 sensor initialized."); // Print success message if sensor initializes
  } else {
    Serial.println("Failed to initialize BH1750 sensor."); // Print error message if sensor fails to initialize
  }

  // Set the LED pin as an output mode
  pinMode(Red_LED, OUTPUT); // Configure the red LED pin as an output
}

void loop() {
  // Check if the sensor is ready to take a measurement
  if (lightMeter.measurementReady(true)) {
    float lux = lightMeter.readLightLevel(); // Read the light level from the sensor
    Serial.print("Current light level: "); // Print the current light level
    Serial.print(lux); // Print the lux value
    Serial.println(" lx"); // Print the unit (lux)

    // If the light level is less than 100 lx, turn on the red LED
    if (lux < 100) {
      digitalWrite(Red_LED, HIGH); // Turn on the red LED
    } else {
      digitalWrite(Red_LED, LOW); // Turn off the red LED
    }
  }

  delay(200); // Wait for 200 milliseconds before checking again
}