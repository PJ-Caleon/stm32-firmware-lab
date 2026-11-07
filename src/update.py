import serial
import time
import os
import zlib
import struct

PORT = '/dev/ttyUSB0'
BAUD = 115200
CHUNK_SIZE = 128  

def send_command_with_retry(ser, cmd_bytes, expected_ack, retries=3, timeout=2):
    ser.timeout = timeout
    for attempt in range(retries):
        ser.reset_input_buffer()
        ser.write(cmd_bytes)
        response = ser.readline()
        if expected_ack in response:
            return True
        print(f"[RETRY {attempt+1}/{retries}] Expected {expected_ack}, got {response.strip() or b'TIMEOUT'}")
    return False

def flash_firmware(kill_at_50_percent=False):
    bin_path = "app.bin"
    if not os.path.exists(bin_path):
        print(f"Error: {bin_path} not found")
        return

    with open(bin_path, "rb") as f:
        app_data = f.read()

    app_size = len(app_data)
    app_crc = zlib.crc32(app_data) & 0xFFFFFFFF
    
    print(f"Firmware loaded: {bin_path} ({app_size} bytes, CRC32: 0x{app_crc:08X})")

    try:
        with serial.Serial(PORT, BAUD, timeout=2) as ser:
            time.sleep(0.5)
            ser.reset_input_buffer()

            # HELLO Handshake
            print("Sending HELLO...")
            if not send_command_with_retry(ser, b"HELLO\n", b"BOOT_V1.0"):
                print("Handshake failed")
                return
            print("Handshake verified")

            # ERASE Command
            print("Sending ERASE...")
            if not send_command_with_retry(ser, b"ERASE\n", b"ACK_ERASE", timeout=5):
                print("Erase command failed")
                return
            print("Flash erased successfully")

            # DATA Transmission
            print("Sending application binary chunks...")
            total_sent = 0
            chunks = [app_data[i:i + CHUNK_SIZE] for i in range(0, app_size, CHUNK_SIZE)]
            total_chunks = len(chunks)

            for idx, chunk in enumerate(chunks):
                # Simulate killing the update at ~50%
                if kill_at_50_percent and idx == total_chunks // 2:
                    print("\n[FAILURE TEST] Abruptly killing update at 50% progress")
                    print("Closing serial port mid-transfer...")
                    return

                # Send chunk header or stream raw bytes
                ser.write(b"DATA\n")
                ser.write(struct.pack("<H", len(chunk)))  # Send chunk length
                ser.write(chunk)                          # Send raw bytes
                
                response = ser.readline()
                if b"ACK_DATA" not in response:
                    print(f"Chunk {idx+1}/{total_chunks} failed. Retrying...")
                
                total_sent += len(chunk)
                progress = (total_sent / app_size) * 100
                print(f"Progress: [{progress:5.1f}%] ({total_sent}/{app_size} bytes)", end="\r")

            print("\nAll data chunks sent.")

            # FINISH Command (Writes metadata last)
            print("Sending FINISH...")
            finish_cmd = f"FINISH {app_size} {app_crc}\n".encode('ascii')
            finish_success = send_command_with_retry(ser, finish_cmd, b"ACK_FINISH")
            if not finish_success:
                print("Finish command failed")
                return
            print("Success! Firmware update completed cleanly")

    except serial.SerialException as e:
        print(f"Serial error: {e}")

if __name__ == "__main__":
    import sys
    simulate_kill = "--kill" in sys.argv
    flash_firmware(kill_at_50_percent=simulate_kill)
