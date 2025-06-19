from machine import Pin, ADC, PWM  # Import Pin, ADC, and PWM classes for hardware control
import time  # Import time module for delays

class SoundBuzzer:
    """
    Sound sensor + buzzer controller:
    - When sound level exceeds threshold, trigger buzzer for a given duration.
    """

    def __init__(self, sound_pin, buzzer_pin, threshold=300, buzzer_freq=1300):
        # Initialize ADC for sound sensor on specified pin
        self.adc = ADC(Pin(sound_pin))
        # Initialize PWM for buzzer on specified pin
        self.buzzer = PWM(Pin(buzzer_pin))
        self.buzzer.freq(buzzer_freq)  # Set buzzer frequency
        self.buzzer.duty_u16(0)        # Start with buzzer off (0% duty cycle)
        self.threshold = threshold      # Set sound detection threshold

    def read_sound(self):
        """Return raw analog value from sound sensor (0–65535)"""
        return self.adc.read_u16()

    def alert(self, duration_ms=3000):
        """Activate buzzer with 50% duty cycle for specified duration (milliseconds)"""
        self.buzzer.duty_u16(32768)  # 50% duty cycle (32768/65535)
        time.sleep_ms(duration_ms)    # Keep buzzer active for duration_ms
        self.buzzer.duty_u16(0)       # Turn off buzzer after duration

    def check_and_alert(self, duration_ms=3000):
        """
        Check if sound level exceeds threshold. If yes, trigger buzzer and return True.
        Otherwise, return False.
        """
        val = self.read_sound()       # Read raw sound sensor value
        if val >= self.threshold:     # Compare with threshold
            self.alert(duration_ms)   # Activate buzzer if threshold is met
            return True
        return False

# === Configuration ===
SOUND_PIN = 29         # ADC pin connected to sound sensor (e.g., GPIO29)
BUZZER_PIN = 10        # PWM pin connected to buzzer (e.g., GPIO10)
THRESHOLD = 5000       # Adjust based on ambient noise (higher = less sensitive)
ALERT_MS = 3000        # Buzzer activation duration in milliseconds
POLL_INTERVAL = 100    # Interval between sound checks in milliseconds

# Initialize SoundBuzzer instance with configured parameters
sb = SoundBuzzer(sound_pin=SOUND_PIN, buzzer_pin=BUZZER_PIN, threshold=THRESHOLD)

print("SoundBuzzer initialized. Monitoring sound...")  # Startup message

try:
    while True:
        if sb.check_and_alert(duration_ms=ALERT_MS):  # Check sound and trigger alert
            print("Sound detected! Buzzer activated for 3 seconds.")
        time.sleep_ms(POLL_INTERVAL)  # Wait for next check interval

except KeyboardInterrupt:
    sb.buzzer.duty_u16(0)  # Ensure buzzer is off on keyboard interrupt
    print("Monitoring stopped. Buzzer deactivated.")  # Shutdown message