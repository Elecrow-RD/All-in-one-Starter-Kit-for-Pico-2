from machine import Pin, ADC
import time

# ========== LED Pin Definitions ==========
# Configure GPIO pins for LEDs as output
red_led = Pin(18, Pin.OUT)
yellow_led = Pin(20, Pin.OUT)
green_led = Pin(19, Pin.OUT)

# ========== ADC Button Configuration ==========
# Define ADC pin for reading button values
ADC_PIN = 27
adc = ADC(Pin(ADC_PIN))

# Measured 12-bit ADC ranges (corresponding to button voltage ranges)
# Each button produces a unique ADC value range when pressed
B0_L, B0_H = 2900, 3000   # Button 0 range → Controls all lights
B1_L, B1_H = 3100, 3300   # Button 1 range → Controls red light
B2_L, B2_H = 3400, 3520   # Button 2 range → Controls yellow light
B3_L, B3_H = 3600, 3700   # Button 3 range → Controls green light

# Initial states for tracking button presses and LED status
prev_key = 0              # Previous detected button
red_on = False            # Red LED status
yellow_on = False         # Yellow LED status
green_on = False          # Green LED status
all_on = False            # All LEDs status

# Main control loop - runs indefinitely
while True:
    # Read 12-bit ADC value (converted from 16-bit raw value)
    raw12 = adc.read_u16() >> 4

    # Determine which button is pressed based on ADC value
    if B0_L <= raw12 <= B0_H:
        key = 0           # Button 0 (All lights)
    elif B1_L <= raw12 <= B1_H:
        key = 1           # Button 1 (Red light)
    elif B2_L <= raw12 <= B2_H:
        key = 2           # Button 2 (Yellow light)
    elif B3_L <= raw12 <= B3_H:
        key = 3           # Button 3 (Green light)
    else:
        key = -1          # No button pressed

    # Detect button press event (rising edge detection)
    if key != -1 and prev_key == -1:
        print(f"Button {key} pressed, ADC value: {raw12}")
        
        # Handle button 0: Toggle all LEDs simultaneously
        if key == 0:
            all_on = not all_on
            red_led.value(all_on)
            yellow_led.value(all_on)
            green_led.value(all_on)
            print("All lights →", "ON" if all_on else "OFF")
            # Synchronize individual LED states with all_on
            red_on = yellow_on = green_on = all_on

        # Handle button 1: Toggle red LED
        elif key == 1:
            red_on = not red_on
            red_led.value(red_on)
            print("Red light →", "ON" if red_on else "OFF")

        # Handle button 2: Toggle yellow LED
        elif key == 2:
            yellow_on = not yellow_on
            yellow_led.value(yellow_on)
            print("Yellow light →", "ON" if yellow_on else "OFF")

        # Handle button 3: Toggle green LED
        elif key == 3:
            green_on = not green_on
            green_led.value(green_on)
            print("Green light →", "ON" if green_on else "OFF")

    # Update previous button state
    prev_key = key
    # Small delay to debounce buttons and reduce CPU usage
    time.sleep_ms(100)