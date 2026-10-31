#ifndef CRC_H
#define CRC_H 

#include <stdint.h>
#include <stddef.h>

// Reference: https://www.embeddedsoft.net/crc-and-checksum-algorithms-in-embedded-c-a-practical-guide/

// CRC-16-CCITT lookup table (256 entries)
extern const uint16_t crc16_ccitt_table[256];

// Function prototypes
uint16_t ccr16_ccitt(const uint8_t *data, size_t length, uint16_t init);
uint16_t crc16_xmodem(const uint8_t *data, size_t length);
uint16_t ccr16_ccitt_false(const uint8_t *data, size_t length);

void crc32_init_sliceby4(void);
uint32_t crc32_sliceby4(const uint8_t *data, size_t len, uint32_t crc);
uint32_t crc32_hardware(const uint8_t *data, size_t len);
uint32_t crc32_bit_by_bit(const uint8_t *data, size_t len, uint32_t crc);

#endif
