from machine import Pin, ADC, PWM  # Import Pin, ADC, and PWM classes for hardware control
import time  # Import time module for delays and timing

class GasBuzzer:
    """
    Gas sensor and buzzer integration.
    Activates buzzer when sensor voltage exceeds threshold; otherwise silent.
    """
    def __init__(self, gas_pin, buzzer_pin, threshold_volt=1.5, vref=3.3, alarm_freq=1300, alarm_duty=0.5):
        """
        gas_pin       : ADC pin connected to gas sensor output (e.g., 26)
        buzzer_pin    : GPIO pin connected to buzzer (e.g., 10)
        threshold_volt: Alarm threshold voltage (V)
        vref          : ADC reference voltage (default: 3.3V)
        alarm_freq    : Buzzer frequency (Hz)
        alarm_duty    : Buzzer duty cycle (0.0 - 1.0)
        """
        self.adc = ADC(Pin(gas_pin))  # Initialize ADC for gas sensor
        self.buzzer = PWM(Pin(buzzer_pin))  # Initialize PWM for buzzer
        self.buzzer.freq(alarm_freq)  # Set buzzer frequency
        self.buzzer.duty_u16(0)  # Start with buzzer off (0% duty cycle)
        self.threshold = threshold_volt  # Store alarm voltage threshold
        self.vref = vref  # Store ADC reference voltage
        self.duty_val = int(65535 * alarm_duty)  # Convert duty cycle to 16-bit value

    def read_voltage(self):
        """
        Read analog value from sensor and convert to voltage (float).
        ADC.read_u16() returns 0–65535, mapped to 0–vref.
        """
        raw = self.adc.read_u16()  # Read raw 16-bit ADC value
        # Actual voltage = Original value x Reference voltage/65535
        return raw * self.vref / 65535  # Convert to voltage (V)

    def update(self):
        """
        Update buzzer state based on latest voltage reading.
        """
        v = self.read_voltage()  # Get current sensor voltage
        if v >= self.threshold:
            # Alarm state: activate buzzer
            self.buzzer.duty_u16(self.duty_val)
        else:
            # Silent state: deactivate buzzer
            self.buzzer.duty_u16(0)
        return v  # Return current voltage for logging

    def deinit(self):
        """Deinitialize buzzer (turn off and release resources)"""
        self.buzzer.duty_u16(0)  # Ensure buzzer is off
        self.buzzer.deinit()  # Deinitialize PWM peripheral


# === Main Program Configuration ===
GAS_PIN       = 26      # Gas sensor GPIO pin (ADC0)
BUZZER_PIN    = 10      # Buzzer GPIO pin
THRESH_VOLT   = 1.5     # Alarm voltage threshold (V)
READ_INTERVAL = 0.5     # Reading interval (seconds)

# Initialize GasBuzzer instance with configured parameters
gb = GasBuzzer(
    gas_pin=GAS_PIN,
    buzzer_pin=BUZZER_PIN,
    threshold_volt=THRESH_VOLT,
    vref=3.3,
    alarm_freq=1300,
    alarm_duty=0.5
)

# Main loop
try:
    while True:
        volt = gb.update()  # Read voltage and update buzzer state
        # Print status message
        print("Gas voltage: {:.2f} V".format(volt),
              "→", "ALARM" if volt >= THRESH_VOLT else "OK")
        time.sleep(READ_INTERVAL)  # Wait for next reading interval

except KeyboardInterrupt:
    gb.deinit()  # Properly deinitialize buzzer on interrupt
    print("Program terminated. Buzzer turned off.")  # Shutdown message