#include <string.h>
#include <unity.h>
#include "uart.h"

static uint32_t dma_remaining = 256;
extern volatile int8_t dma_rx_buffer[256];
extern volatile uint16_t rx_tail;

void setUp(){
  rx_tail = 0;
}
void tearDown(){}

void rx_empty_buffer_pop(void){
  dma_remaining = 256; // head = 0, tail = 0 (empty)
  dma_rx_buffer[rx_tail] = 'A';

  TEST_ASSERT_EQUAL_CHAR('\0', uart_read_char());
}



int main(void){
  UNITY_BEGIN();
  RUN_TEST(rx_empty_buffer_pop);
  return UNITY_END();
}

/*
  Empty buffer: pop fails cleanly 

  Push then pop returns the same byte 

  fill it 100%: next push should reject 

  Wrap Around: if index is 255, pushing another goes back to index 0
*/

void dma1_init(void){}
void dma1_channel_config(uint8_t channel, uint32_t periph_addr, uint32_t mem_addr, uint16_t size, uint32_t ccr) {
    (void)channel; (void)periph_addr; (void)mem_addr; (void)size; (void)ccr;
}
void dma1_channel_enable(uint8_t channel) { (void)channel; }
void dma1_set_tc_callback(uint8_t channel, void (*callback)(void)) { (void)channel; (void)callback; }
uint16_t dma1_channel_remaining(uint8_t channel) {
    (void)channel;
    return dma_remaining;
}
