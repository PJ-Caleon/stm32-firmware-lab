#include "crc.h"
#include "stm32f103.h"

void crc32_init_sliceby4(void) {
}

uint16_t ccr16_ccitt(const uint8_t *data, size_t length, uint16_t init){
  uint16_t crc = init;
  for (size_t i = 0; i < length; i++){
    crc ^= ((uint16_t)data[i] << 8);
    for (uint8_t j = 0; j < 8; j++){
      if (crc & 0x8000){
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

uint16_t crc16_xmodem(const uint8_t *data, size_t length){
  return ccr16_ccitt(data, length, 0x0000);
}

uint16_t ccr16_ccitt_false(const uint8_t *data, size_t length){
  return ccr16_ccitt(data, length, 0xFFFF);
}

uint32_t crc32_bit_by_bit(const uint8_t *data, size_t len, uint32_t init_crc){
  uint32_t crc = ~init_crc;
  for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return ~crc;
}

uint32_t crc32_sliceby4(const uint8_t *data, size_t len, uint32_t crc) {
    return crc32_bit_by_bit(data, len, crc);
}

__attribute__((weak)) uint32_t crc32_hardware(const uint8_t *data, size_t len) {
    SET_BIT(RCC->AHBENR, (RCC_AHBENR_CRCEN));
    F1_CRC->CR = 1;

    size_t word_count = len / 4;
    const uint32_t *word_data = (const uint32_t *)data;

    for (size_t i = 0; i < word_count; i++) {
        F1_CRC->DR = word_data[i];
    }

    size_t processed = word_count * 4;
    if (processed < len) {
        uint32_t last_word = 0;
        for (size_t i = processed; i < len; i++) {
            ((uint8_t *)&last_word)[i - processed] = data[i];
        }
        F1_CRC->DR = last_word;
    }

    return F1_CRC->DR;
}
