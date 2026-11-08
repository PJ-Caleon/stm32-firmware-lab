#include "cobs.h"

size_t cobs_encode(const uint8_t *input, size_t length, uint8_t *output) {
    size_t read_idx = 0;
    size_t write_idx = 1;
    size_t code_idx = 0;
    uint8_t code = 1;

    while (read_idx < length) {
        if (input[read_idx] == 0) {
            output[code_idx] = code;
            code = 1;
            code_idx = write_idx++;
            read_idx++;
        } else {
            output[write_idx++] = input[read_idx++];
            code++;
            if (code == 0xFF) {
                output[code_idx] = code;
                code = 1;
                code_idx = write_idx++;
            }
        }
    }
    output[code_idx] = code;
    output[write_idx++] = 0x00; // Frame delimiter 
    return write_idx;
}

int cobs_decode(const uint8_t *input, size_t length, uint8_t *output, size_t *out_len) {
    size_t read_idx = 0;
    size_t write_idx = 0;

    if (length == 0) {
        return -1;
    }

    while (read_idx < length) {
        uint8_t code = input[read_idx];
        if (code == 0) {
            return -1;
        }
        read_idx++;

        for (uint8_t i = 1; i < code; i++) {
            if (read_idx >= length) {
                return -1;
            }
            output[write_idx++] = input[read_idx++];
        }

        if (code < 0xFF && read_idx < length) {
            output[write_idx++] = 0;
        }
    }

    *out_len = write_idx;
    return 0;
}
