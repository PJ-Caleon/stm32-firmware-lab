#include "parser.h"
#include "cobs.h"
#include "crc.h"

void parser_init(packet_parser *parser){
  parser->index = 0;
}

int parser_feed(packet_parser *parser, uint8_t byte, decode_parser *pkt) {
    if (byte == 0x00) {
        if (parser->index == 0) {
            return 0; // Ignore consecutive frame delimiter bytes
        }

        uint8_t decoded_buf[128];
        size_t decoded_len = 0;

        // Decode the COBS-framed packet
        if (cobs_decode(parser->buffer, parser->index, decoded_buf, &decoded_len) != 0) {
            parser->index = 0;
            return -1; // Decode error, resynchronize state machine
        }

        parser->index = 0;

        // Minimum length check: Type(1) + Seq(1) + Len(1) + CRC16(2) = 5 bytes
        if (decoded_len < 5) {
            return -2;
        }

        uint8_t msg_type = decoded_buf[0];
        uint8_t seq      = decoded_buf[1];
        uint8_t pay_len  = decoded_buf[2];

        if (decoded_len != (size_t)(3 + pay_len + 2)) {
            return -3; // Length field mismatch
        }

        // Extract and verify CRC16 using your existing ccitt_false function
        uint16_t rx_crc = (uint16_t)decoded_buf[3 + pay_len] | 
                          ((uint16_t)decoded_buf[3 + pay_len + 1] << 8);
        
        uint16_t calc_crc = ccr16_ccitt_false(decoded_buf, 3 + pay_len);

        if (rx_crc != calc_crc) {
            return -4; // CRC validation failed
        }

        // Populate successfully parsed packet data
        pkt->type = msg_type;
        pkt->seq = seq;
        pkt->length = pay_len;
        for (size_t i = 0; i < pay_len; i++) {
            pkt->payload[i] = decoded_buf[3 + i];
        }

        return 1;
    }

    // Accumulate bytes into parser buffer with overflow protection
    if (parser->index < sizeof(parser->buffer)) {
        parser->buffer[parser->index++] = byte;
    } else {
        parser->index = 0; // Reset state machine if stream runs away
    }

    return 0;
}
