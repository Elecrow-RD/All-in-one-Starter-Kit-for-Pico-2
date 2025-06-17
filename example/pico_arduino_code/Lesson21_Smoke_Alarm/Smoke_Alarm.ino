#define gas_pin 26         // The pin where the MQ2 analog output is connected
const int buzzerPin = 10;  // The pin where the buzzer is connected

void setup() {
  pinMode(buzzerPin, OUTPUT);  // Configure the buzzer pin as an output
  Serial.begin(115200);        // Initialize serial communication at 115200 baud rate

  Serial.println("MQ2 ready!");  // Print initialization complete message
}

void loop() {
  float sensorValue = analogRead(gas_pin);               // Read the analog value from the sensor
  float sensor_volt = sensorValue / 1023.0 * 5.0;        // Convert the analog value to voltage (assuming 5V reference)

  Serial.print("Voltage: ");  // Print voltage label
  Serial.print(sensor_volt);  // Print voltage value
  Serial.println(" V");       // Print voltage unit

  if (sensor_volt > 1.0) {          // If the detected voltage exceeds 1.0V, smoke is detected
    tone(buzzerPin, 1300);          // Activate the buzzer with a 1300Hz tone
  } else {
    noTone(buzzerPin);              // Otherwise, turn off the buzzer
  }

  delay(100); // Control the detection frequency
}