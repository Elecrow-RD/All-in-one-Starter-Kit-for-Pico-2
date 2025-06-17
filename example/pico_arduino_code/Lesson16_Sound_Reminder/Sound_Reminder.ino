#define SOUND_PIN 29 // Define the pin connected to the sound sensor
int buzzerPin = 10; // Define the pin connected to the buzzer

void setup() {
  pinMode(SOUND_PIN, INPUT); // Set the sound sensor pin as an input
  pinMode(buzzerPin, OUTPUT); // Set the buzzer pin as an output
  Serial.begin(9600); // Initialize serial communication at 9600 baud rate
  Serial.println("Sound detection started..."); // Print a startup message to the serial monitor
}

void loop() {
  int soundValue = analogRead(SOUND_PIN); // Read the analog value from the sound sensor
  Serial.print("Sound Value: "); // Print the sound value to the serial monitor
  Serial.println(soundValue);

  if (soundValue >= 300) { // Check if the sound value is above a certain threshold (300 in this case)
    Serial.println("Loud sound detected! Beep!"); // Print a message indicating a loud sound was detected
    tone(buzzerPin, 1300); // Generate a tone on the buzzer pin at a frequency of 1300 Hz
    delay(1000);  // Keep the buzzer on for 1 second
  } else {
    Serial.println("Quiet. Buzzer off."); // Print a message indicating it's quiet
    noTone(buzzerPin); // Stop the buzzer
  }

  delay(100);  // Short delay to avoid overly frequent readings
}