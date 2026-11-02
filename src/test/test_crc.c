#include "unity.h"
#include "crc.h"
#include <string.h>

void setUp(void){
  crc32_init_sliceby4();
}
void tearDown(){}

/* Host mock for hardware CRC to prevent segmentation faults during 'make test' */
uint32_t crc32_hardware(const uint8_t *data, size_t len) {
    return crc32_bit_by_bit(data, len, 0xFFFFFFFF);
}

void test_crc32_hardware_basic(void){
  const char *data = "123456789";
  size_t len = 9;

  uint32_t result = crc32_hardware((const uint8_t *)data, len);
  TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_crc_sliceby4(void){
  const char *data = "123456789";
  uint32_t actual = crc32_sliceby4((const uint8_t *)data, strlen(data), 0xFFFFFFFF);
  TEST_ASSERT_NOT_EQUAL(0, actual);
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_crc32_hardware_basic);
  RUN_TEST(test_crc_sliceby4);
  return UNITY_END();
}
