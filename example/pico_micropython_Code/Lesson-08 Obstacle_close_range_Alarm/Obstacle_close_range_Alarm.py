from machine import Pin, time_pulse_us  # Import Pin and time_pulse_us from machine module for GPIO control and pulse duration measurement
import time  # Import time module for delay operations

# Define pins
trigger_pin = Pin(9, Pin.OUT)     # Set GPIO9 as output mode, connected to the Trigger pin of the ultrasonic sensor to emit signals
echo_pin = Pin(8, Pin.IN)         # Set GPIO8 as input mode, connected to the Echo pin of the ultrasonic sensor to receive signals
vibrate_pin = Pin(15, Pin.OUT)    # Set GPIO15 as output mode, connected to the vibration motor control pin

# Single distance measurement function (unit: cm)
def measure_distance_cm():
    trigger_pin.off()                 # Ensure the trigger pin starts low
    time.sleep_us(2)                 # Wait 2 microseconds for stability
    trigger_pin.on()                 # Set the trigger pin high
    time.sleep_us(10)               # Keep high for 10 microseconds to trigger the ultrasonic signal
    trigger_pin.off()                # Pull the trigger pin low again to complete the pulse

    try:
        # Measure how long the echo pin stays high; wait up to 60 ms to avoid blocking
        duration = time_pulse_us(echo_pin, 1, 60000)
    except OSError:
        return -1  # Return -1 if timeout or error occurs

    # Calculate distance: duration (µs) × speed (0.0343 cm/µs) ÷ 2 (round trip)
    distance = (duration * 0.0343) / 2
    return distance  # Return the measured distance in centimeters

# Measure multiple times and return the average for better stability
def average_distance(samples=3):
    distances = []  # Store valid distance measurements
    for _ in range(samples):  # Repeat for the given number of samples
        d = measure_distance_cm()  # Call single measurement function
        if d > 0:                  # If measurement is valid
            distances.append(d)   # Add to list
        time.sleep_ms(50)         # Wait 50 ms between measurements
    # Return average if valid data exists, otherwise return -1
    return sum(distances) / len(distances) if distances else -1

# Main program entry
def main():
    vibrate_pin.off()  # Turn off vibration motor at startup to ensure initial state
    print("Starting ultrasonic distance monitoring...")  # Print startup message

    while True:  # Infinite loop to continuously check distance
        distance = average_distance()  # Get average distance value
        if distance > 0:  # If measurement is valid
            print("Distance: {:.2f} cm".format(distance))  # Print current distance with 2 decimal places
            if distance < 30:  # If distance is less than 30 cm, object is near
                vibrate_pin.on()  # Turn on vibration motor
            else:  # If distance is 30 cm or more, it's safe
                vibrate_pin.off()  # Turn off vibration motor
        else:
            print("Measurement error, retrying...")  # Print error message if measurement fails

        time.sleep(0.2)  # Wait 200 ms between each measurement cycle to avoid high frequency

# Run the main program
main()
