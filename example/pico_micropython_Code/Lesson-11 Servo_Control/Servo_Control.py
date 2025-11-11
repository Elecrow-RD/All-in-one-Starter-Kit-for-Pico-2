from machine import Pin, PWM    # Import the Pin module for GPIO pin control and the PWM module for generating pulse width modulation signals
import time                     # Import the time module to implement delay functions

class Servo:
    """
    RC servo motor controller for mapping 0–180° angles to specific pulse widths (suitable for standard servo control)
    """
    def __init__(self, pin_num, freq=50, min_us=600, max_us=2400):
        """
        pin_num: GPIO pin number connected to the servo signal wire (e.g., 13)
        freq: PWM signal frequency (in Hz), standard servos typically use 50Hz (corresponding to a 20ms period)
        min_us: Pulse width (in microseconds) corresponding to 0° rotation of the servo
        max_us: Pulse width (in microseconds) corresponding to 180° rotation of the servo
        """
        self.pwm = PWM(Pin(pin_num))          # Create a PWM object and associate it with the specified GPIO pin
        self.pwm.freq(freq)                    # Set the PWM frequency to the specified value
        self.min_us = min_us                    # Store the minimum pulse width corresponding to 0°
        self.max_us = max_us                    # Store the maximum pulse width corresponding to 180°
        self.period = 1000000 // freq           # Calculate the PWM period (in microseconds), where 1 second = 1,000,000 microseconds

    def _angle_to_duty(self, angle):
        """
        Map 0–180° angles to the [min_us, max_us] pulse width range and convert them to a PWM duty cycle of 0–65535
        """
        angle = max(0, min(180, angle))         # Limit the angle range between [0°, 180°] to prevent out-of-bounds values
        us = self.min_us + (self.max_us - self.min_us) * (angle / 180)  # Linearly map the angle to the pulse width
        duty = int((us / self.period) * 65535)   # Convert the pulse width to a PWM duty cycle (16-bit unsigned integer)
        return duty

    def write_angle(self, angle):
        """
        Control the servo to rotate to the specified angle (0–180°)
        """
        duty = self._angle_to_duty(angle)        # Get the PWM duty cycle corresponding to the angle
        self.pwm.duty_u16(duty)                  # Set the PWM duty cycle (16-bit value)
        time.sleep_ms(50)                         # Delay for 50 milliseconds to allow the servo to complete the rotation

    def deinit(self):
        self.pwm.deinit()                         # Release PWM resources and stop signal output


# === Configuration Parameters ===
SERVO_PIN = 13                # Define the GPIO pin connected to the servo signal wire as 13
SWEEP_DELAY = 1000            # Delay time (in milliseconds) when the servo reaches both ends (0° and 180°)
LOOP_PAUSE = 1500             # Pause time (in milliseconds) after completing a full round-trip sweep

# Initialize the servo object
servo = Servo(pin_num=SERVO_PIN, freq=50, min_us=600, max_us=2400)

try:
    while True:
        # Sweep from 0° to 180° gradually (increasing by 5° each time)
        for angle in range(0, 181, 5):
            servo.write_angle(angle)            # Control the servo to rotate to the current angle
        
        time.sleep_ms(SWEEP_DELAY)              # Stay at the 180° position for the specified time

        # Sweep from 180° to 0° gradually (decreasing by 5° each time)
        for angle in range(180, -1, -5):
            servo.write_angle(angle)            # Control the servo to rotate to the current angle
        
        time.sleep_ms(SWEEP_DELAY)              # Stay at the 0° position for the specified time

        # Pause after completing a full sweep cycle
        time.sleep_ms(LOOP_PAUSE)

except KeyboardInterrupt:
    servo.deinit()                              # Capture keyboard interrupt (Ctrl+C) and release servo resources
    print("Servo control stopped.")             # Print a stop prompt