import serial
import time

# Update with your board's actual serial port (e.g., '/dev/ttyUSB0', '/dev/ttyACM0', or 'COM3')
PORT = '/dev/ttyUSB0' 
BAUD = 115200

print(f"Connecting to {PORT} at {BAUD} baud...")
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2) # Allow board to reset

# Clear any old data in buffers
ser.reset_input_buffer()
ser.reset_output_buffer()

print("Sending 100,000 bytes counter pattern...")
# Generate 100,000 bytes with a rolling counter (0 to 255 repeating)
chunk_size = 1024
total_bytes = 100000
sent = 0

start_time = time.time()

while sent < total_bytes:
    remaining = total_bytes - sent
    current_chunk_size = min(chunk_size, remaining)
    
    # Create counter pattern block
    data = bytes([(i % 256) for i in range(sent, sent + current_chunk_size)])
    
    ser.write(data)
    sent += current_chunk_size

elapsed = time.time() - start_time
print(f"Finished sending {total_bytes} bytes in {elapsed:.2f} seconds.")
print("Waiting for MCU response (error count)...")

# Listen for output from the MCU for a few seconds
timeout_counter = time.time() + 5
while time.time() < timeout_counter:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line:
            print(f"MCU Response: {line}")

ser.close()
print("Stress test complete.")
