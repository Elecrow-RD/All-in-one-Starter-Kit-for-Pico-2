from st77xx import *
import ft6x36
import machine
import fs_driver
import sys
import lvgl as lv
sys.path.append('.')

spi=machine.SPI(
    0,
    baudrate=24_000_000,
    polarity=0,
    phase=0,
    sck=machine.Pin(6, machine.Pin.OUT),
    mosi=machine.Pin(7, machine.Pin.OUT),
    miso=None
)
# dma=rp2_dma.DMA(0)
rp2_dma=None

lcd =St7789(rot=1, res=(240,320), spi=spi,rp2_dma=rp2_dma,cs=17,dc=16,bl=0,bgr=True,rst=None)

# bl
p0 = machine.Pin(0, machine.Pin.OUT)
p0(1)

i2c = machine.I2C(0, scl=machine.Pin(5), sda=machine.Pin(4))
touch = ft6x36.FT6x36()

fs_drv = lv.fs_drv_t()
fs_driver.fs_register(fs_drv, 'S')

# =================================================================================

import lvgl as lv  # Import the lvgl library for graphical user interface
import time  # Import the time module for timing functions
import ui  # Import the ui module for user interface elements
from ir_polling import IRDecoder  # Import the IRDecoder class for infrared signal decoding

# Global variables
input_text = ""  # Variable to store the current input expression
evaluated = False  # Flag to indicate if the expression has been evaluated
ir_decoder = None  # Variable to store the IRDecoder instance
last_key_time = 0  # Variable to store the last key press time for debouncing

# Functions related to expression evaluation
def precedence(op):
    # Determine the precedence of an operator
    if op in ('+', '-'):
        return 1  # Lower precedence for addition and subtraction
    if op in ('*', '/'):
        return 2  # Higher precedence for multiplication and division
    return 0  # Default precedence for non-operators

def is_operator(c):
    # Check if a character is an operator
    return c in ('+', '-', '*', '/')

def infix_to_postfix(infix):
    # Convert an infix expression to postfix notation
    output = ""  # Output string for the postfix expression
    stack = []  # Stack to hold operators
    num = ""  # Temporary string to build numbers
    
    for i, c in enumerate(infix):
        if c == ' ':
            continue  # Skip spaces
            
        if c == '-' and (i == 0 or is_operator(infix[i-1])):
            num += c  # Handle unary minus
            continue
            
        if c.isdigit() or c == '.':  # If the character is a digit or decimal point
            num += c
        elif is_operator(c):  # If the character is an operator
            if num:
                output += num + " "  # Add the number to the output
                num = ""
                
            while stack and precedence(stack[-1]) >= precedence(c):  # Maintain operator precedence
                output += stack.pop() + " "
            stack.append(c)
        else:
            return None  # Invalid character
            
    if num:
        output += num + " "  # Add the last number to the output
        
    while stack:  # Add remaining operators to the output
        output += stack.pop() + " "
        
    return output

def evaluate_postfix(postfix):
    # Evaluate a postfix expression
    if postfix is None:
        return float('nan')  # Return NaN if the postfix expression is invalid
        
    stack = []  # Stack to hold operands
    tokens = postfix.split()  # Split the postfix expression into tokens
    
    for token in tokens:
        if token and is_operator(token[0]) and len(token) == 1:  # If the token is an operator
            if len(stack) < 2:
                return float('nan')  # Not enough operands
            b = stack.pop()
            a = stack.pop()
            if token == '+':
                stack.append(a + b)
            elif token == '-':
                stack.append(a - b)
            elif token == '*':
                stack.append(a * b)
            elif token == '/':
                if b == 0:
                    return float('nan')  # Division by zero
                stack.append(a / b)
        else:
            try:
                stack.append(float(token))  # Push operand to stack
            except ValueError:
                return float('nan')  # Invalid operand
                
    if len(stack) == 1:
        return stack[0]  # Return the result
    return float('nan')  # Invalid expression

def evaluate_expression(expr):
    # Evaluate an infix expression
    postfix = infix_to_postfix(expr)  # Convert to postfix
    if postfix is None:
        return float('nan')  # Invalid expression
    return evaluate_postfix(postfix)  # Evaluate the postfix expression

# Update the display with the given text
def update_display(text):
    ui.ui_Label1.set_text(text)  # Set the text of the display label

# Process an infrared key press
def process_ir_key(code):
    global input_text, evaluated, last_key_time
    
    current_time = time.ticks_ms()  # Get the current time in milliseconds
    # Simple debouncing to prevent repeated key presses
    if time.ticks_diff(current_time, last_key_time) < 50:
        return
    last_key_time = current_time
    
    print(f"Received IR code: 0x{code:02X}")  # Print the received IR code
    
    # If the expression has been evaluated, clear the input
    if evaluated:
        input_text = ""
        evaluated = False
    
    # Map IR codes to keys
    key_map = {
        0x16: "0",
        0x0C: "1",
        0x18: "2",
        0x5E: "3",
        0x08: "4",
        0x1C: "5",
        0x5A: "6",
        0x42: "7",
        0x52: "8",
        0x4A: "9",
        0x15: "+",
        0x07: "-",
        0x44: "/",
        0x40: "*",
        0x09: "",  # Clear input
        0x43: "=",  # Evaluate expression
        0x19: "."   # Decimal point
    }
    
    if code in key_map:
        key = key_map[code]
        
        if key == "":
            input_text = ""  # Clear the input
        elif key == "=":
            result = evaluate_expression(input_text)  # Evaluate the expression
            if float('nan') == result:
                update_display("Error")  # Display error
            elif not float('inf') == result and not float('-inf') == result:
                update_display(f"Result: {result:.2f}")  # Display the result
            else:
                update_display("Overflow")  # Display overflow error
            evaluated = True
        elif key == ".":
            # Prevent consecutive decimal points or placing a decimal point after an operator
            if (not input_text or input_text.endswith(".") or 
                (input_text and is_operator(input_text[-1]))):
                pass
            else:
                # Check if the current number already has a decimal point
                i = len(input_text) - 1
                while i >= 0 and (input_text[i].isdigit() or input_text[i] == '.'):
                    if input_text[i] == '.':
                        break
                    i -= 1
                if i < 0 or input_text[i] != '.':
                    input_text += key
        else:
            input_text += key  # Append the key to the input
        
        if not evaluated:
            update_display(f"Input: {input_text}")  # Update the display with the current input

# Initialize the infrared receiver
def init_ir_receiver(pin_no=11):
    global ir_decoder
    ir_decoder = IRDecoder(pin_no)  # Create an IRDecoder instance
    print(f"IR decoder initialized on pin {pin_no}")
    return ir_decoder

# Main program
def main():
    # Initialize LVGL
    lv.init()
    
    # Initialize the infrared receiver
    init_ir_receiver()
    
    # Main loop
    try:
        while True:
            # Poll for infrared signals
            code = ir_decoder.read()
            if code is not None:
                process_ir_key(code)  # Process the received IR code
            
            # Handle LVGL tasks to ensure the interface is updated
            lv.task_handler()
            time.sleep_ms(5)  # Reduce delay to improve responsiveness
    except KeyboardInterrupt:
        print("Program terminated")  # Handle program termination

if __name__ == "__main__":
    main()  # Start the main program