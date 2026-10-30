#include "unity.h"
#include <stdint.h>
#include "clock.h"

extern volatile uint32_t ms_tick;
static uint32_t virt_tick = 0;
static int poll_count = 0;

void setUp(void){
  ms_tick = 0;
  virt_tick = 0;
  poll_count = 0;
}
void tearDown(void){
  ms_tick = 0;
}

// Getter function from clock.c override
uint32_t get_ms_tick(void){
  poll_count++;
  if (poll_count > 2){
    virt_tick += 5;
  }
  return virt_tick;
}

void test_millis(void){
  ms_tick = 10;

  for (int i = 0; i < 5; i++){
    SysTick_Handler();
  }
  TEST_ASSERT_EQUAL_INT(15, millis());
}

void test_delay(void){
  virt_tick = 100;
  poll_count = 0;

  delay_ms(5);
  TEST_ASSERT_TRUE(poll_count > 2);
  TEST_ASSERT_EQUAL_INT32(105, virt_tick);
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_millis);
  RUN_TEST(test_delay);
  return UNITY_END();
}
