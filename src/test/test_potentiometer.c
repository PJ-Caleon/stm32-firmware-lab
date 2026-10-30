#include "unity.h"
#include "potentiometer.h"

static uint16_t pot_raw_volt;

void setUp() {
}
void tearDown() {
}

// Override 
uint16_t ADC_GetScanPotRaw(void){
  return pot_raw_volt;
}

void test_Conversion_Case_Low(void) {
    pot_raw_volt = 67;
    TEST_ASSERT_EQUAL_INT(53, Potentiometer_ReadRaw());
}

void test_Conversion_Case_Mid(void) {
    pot_raw_volt = 500;
    TEST_ASSERT_EQUAL_INT(402, Potentiometer_ReadRaw());
}

void test_Conversion_Case_High(void) {
    pot_raw_volt = 4240;
    TEST_ASSERT_EQUAL_INT(3416, Potentiometer_ReadRaw());
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_Conversion_Case_Low);
  RUN_TEST(test_Conversion_Case_Mid);
  RUN_TEST(test_Conversion_Case_High);
  return UNITY_END();
}

/*
  We cannot mock ADC1 specifically because ADC1 takes in analog data
  and the stm32f103 does the conversion internally 
  therefore the output from ADC1 to DR is already the digital.
  Instead we mock the adc_dma_buffer[0]
*/
