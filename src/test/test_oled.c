#include <string.h>
#include "unity.h"
#include "oled.h"

extern uint8_t framebuffer[1024];
extern uint8_t chunk_packet[65];

void setUp(){
  // Input the buffer with data 
  memset(framebuffer, 0, sizeof(framebuffer));
  chunk_packet[0] = 0x40;
}
void tearDown(){
  memset(framebuffer, 0, sizeof(framebuffer));
  chunk_packet[0] = 0x40;
}

void test_oled_clear(void){
  memset(framebuffer, 0xFF, sizeof(framebuffer));

  oled_clear();
  
  TEST_ASSERT_EQUAL_INT(0, (framebuffer[0]));
  TEST_ASSERT_EQUAL_INT(0, (framebuffer[500]));
  TEST_ASSERT_EQUAL_INT(0, (framebuffer[1023]));
} 

void test_oled_draw_pixel1(void){
  oled_draw_pixel(128,64,0);
  TEST_ASSERT_EQUAL_INT(0, (framebuffer[1152]));
}

void test_oled_draw_pixel2(void){
  oled_draw_pixel(100,50,1);
  TEST_ASSERT_EQUAL_HEX(0x04, (framebuffer[868]));
  oled_draw_pixel(50,63,1);
  TEST_ASSERT_EQUAL_HEX(0x80, (framebuffer[946]));
}

void test_oled_draw_pixel3(void){
  framebuffer[868] = 0x24;
  oled_draw_pixel(100,50,0); // the clear is ~0x04
  TEST_ASSERT_EQUAL_HEX(0x20, (framebuffer[868]));
  
  framebuffer[946] = 0x8C;
  oled_draw_pixel(50,63,0); // the clear is ~0x80
  TEST_ASSERT_EQUAL_HEX(0x0C, (framebuffer[946]));
}

void test_oled_refresh(void){
  chunk_packet[53] = 0xFF;
  oled_refresh();
  TEST_ASSERT_EQUAL_HEX(0x00, chunk_packet[53]);
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_oled_clear);
  RUN_TEST(test_oled_draw_pixel1);
  RUN_TEST(test_oled_draw_pixel2);
  RUN_TEST(test_oled_draw_pixel3);
  RUN_TEST(test_oled_refresh);
  return UNITY_END();
}

/*
  OLED Framebuffer logic

  Cannot call the other functions directly because the 
  architecture of the PC and the STM is different

  Since the drivers are created in Bare Metal C, the PC 
  will not recognize stuff like GPIO so we just mock up the functions
  that the compiler needs 
*/

// Mocks for linker 

int i2c_write(uint8_t addr, const uint8_t *data, uint16_t len){
  (void)addr; (void)data; (void)len;
  return 0;
}

unsigned long millis(void){
  return 0;
}

void draco_malfoid(void){}
