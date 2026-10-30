#include "unity.h"
#include "add.h"

void setUp() {}
void tearDown() {}

void test_adding(void){
  TEST_ASSERT_EQUAL_INT(10, add(4,6));
  TEST_ASSERT_EQUAL_INT(12, add(4,8));
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_adding);
  return UNITY_END();
}

