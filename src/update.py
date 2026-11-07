import serial 
import time 

PORT = '/dev/ttyUSB0'
BAUD = 115200

def run_handshake():
    try:
        with serial.Serial(PORT, BAUD, timeout=2) as ser:
            time.sleep(0.5)
            ser.reset_input_buffer()
            print("Sending Hello")
            ser.write(b"HELLO\n")
            response = ser.readline()

            if response:
                if b"BOOT_V1.0" in response:
                    print("Handshake Verified")
                else:
                    print("Unexpected Format")
            else:
                print("Timeout")
    except serial.SerialException as e:
        print(f"Serial Error: {e}")
if __name__ == "__main__":
    run_handshake()
