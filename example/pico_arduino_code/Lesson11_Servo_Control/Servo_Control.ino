#include <Servo.h> // Include the Servo library to control a servo motor

Servo myservo; // Create a servo object to control a servo

void setup() {
  myservo.attach(13, 450, 2520);  // Attach the servo to pin 13 and set the pulse width range (450 to 2520 microseconds)
}

void loop() {
  // Move from 0 degrees to 180 degrees
  for (int pos = 0; pos <= 180; pos++) { // Loop from 0 to 180 degrees
    myservo.write(pos); // Set the servo angle to the current position
    delay(15);  // Wait for 15 milliseconds to control the speed of rotation
  }

  // Move from 180 degrees back to 0 degrees
  for (int pos = 180; pos >= 0; pos--) { // Loop from 180 to 0 degrees
    myservo.write(pos); // Set the servo angle to the current position
    delay(15); // Wait for 15 milliseconds to control the speed of rotation
  }
}