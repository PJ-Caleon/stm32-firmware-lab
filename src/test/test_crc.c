#include "unity.h"
#include "crc.h"
#include <string.h>

void setUp(){
  crc32_init_sliceby4();
}
void tearDown(){}

void crc16_ccitt_false_standard_vector(void){
  const char *data = "123456789";
  uint16_t expected_crc = 0x29B1;

  // Call function with 0xFFFF init which tells to use ccitt_false 
  uint16_t actual_crc = ccr16_ccitt_false((const uint8_t *)data, strlen(data));

  TEST_ASSERT_EQUAL_HEX16(expected_crc, actual_crc);
}

void crc32_ccitt_standard_vector(void){
  const char *data = "123456789";
  uint32_t expected_crc = 0xD202D277;
  uint32_t init_val = 0xFFFFFFFF;

  uint32_t actual_crc = crc32_sliceby4((const uint8_t *)data, strlen(data), init_val);

  TEST_ASSERT_EQUAL_HEX32(expected_crc, actual_crc);
}

void crc32_bit_flip_and_table_vs_bitbybit_match(void){
  uint8_t data1[] = "Hello";
  uint8_t data2[] = "Hello";
  data2[0] ^= 0x01; // Flip a single bit 

  uint32_t init_val = 0xFFFFFFFF;
  size_t len = sizeof(data1) - 1;

  uint32_t crc_orig = crc32_sliceby4(data1, len, init_val);
  uint32_t crc_flipped = crc32_sliceby4(data2, len, init_val);
  TEST_ASSERT_NOT_EQUAL(crc_orig, crc_flipped);

  uint32_t crc_table = crc32_sliceby4(data1, len, init_val);
  uint32_t crc_bitbybit = crc32_bit_by_bit(data1, len, init_val);
  TEST_ASSERT_EQUAL_HEX32(crc_bitbybit, crc_table);
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(crc16_ccitt_false_standard_vector);
  RUN_TEST(crc32_ccitt_standard_vector);
  RUN_TEST(crc32_bit_flip_and_table_vs_bitbybit_match);
  return UNITY_END();
}
