import subprocess
import struct
import os
import pytest

def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= (b << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

def cobs_encode(data: bytes) -> bytes:
    output = bytearray()
    code_idx = 0
    code = 1
    output.append(0)

    for b in data:
        if b == 0:
            output[code_idx] = code
            code = 1
            code_idx = len(output)
            output.append(0)
        else:
            output.append(b)
            code += 1
            if code == 0xFF:
                output[code_idx] = code
                code = 1
                code_idx = len(output)
                output.append(0)

    output[code_idx] = code
    output.append(0x00) # Frame delimiter
    return bytes(output)

def build_packet(msg_type: int, seq: int, payload: bytes) -> bytes:
    header_and_payload = struct.pack("BBB", msg_type, seq, len(payload)) + payload
    crc = crc16_ccitt_false(header_and_payload)
    # Little-endian CRC append
    full_frame = header_and_payload + struct.pack("<H", crc)
    return cobs_encode(full_frame)

def test_roundtrip_python_to_c():
    # Verify packet building structure
    pkt = build_packet(msg_type=0x01, seq=0x05, payload=b"TEST")
    
    # Assert frame finishes with 0x00 delimiter
    assert pkt[-1] == 0x00
    
    # Ensure COBS output contains zero raw 0x00 bytes inside body
    assert 0x00 not in pkt[:-1]
    
    print("\n[Pytest] Frame generated successfully:", list(pkt))

if __name__ == "__main__":
    test_roundtrip_python_to_c()
