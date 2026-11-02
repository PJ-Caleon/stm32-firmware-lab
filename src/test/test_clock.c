#include "unity.h"
#include <stdint.h>
#include "clock.h"

typedef uint32_t TickType_t;

static uint32_t simulated_ticks = 0;
static uint32_t delayed_ms = 0;

uint32_t get_ms_tick(void);

uint32_t xTaskGetTickCount(void) {
  return simulated_ticks;
}

void vTaskDelay(TickType_t xTicksToDelay) {
  delayed_ms += xTicksToDelay;
  simulated_ticks += xTicksToDelay;
}

void setUp(void){
  simulated_ticks = 0;
  delayed_ms = 0;
}

void tearDown(void){
}

void test_millis(void){
  simulated_ticks = 1234;
  TEST_ASSERT_EQUAL_UINT32(1234, millis());
  TEST_ASSERT_EQUAL_UINT32(1234, get_ms_tick());
}

void test_delay(void){
  simulated_ticks = 100;
  delay_ms(25);
  TEST_ASSERT_EQUAL_UINT32(125, simulated_ticks);
  TEST_ASSERT_EQUAL_UINT32(25, delayed_ms);
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_millis);
  RUN_TEST(test_delay);
  return UNITY_END();
}
