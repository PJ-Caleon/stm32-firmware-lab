#include <string.h>
#include <unity.h>
#include "uart.h"

static uint32_t dma_remaining = 256;
extern volatile int8_t dma_rx_buffer[256];
extern volatile uint16_t rx_tail;
extern volatile uint16_t tx_head;
extern volatile uint16_t tx_tail;

void setUp(){
  rx_tail = 0;
  tx_head = 0;
  tx_tail = 0;
  dma_remaining = 256;
}
void tearDown(){}

void rx_empty_buffer_pop(void){
  dma_remaining = 256; // head = 0, tail = 0 (empty)
  dma_rx_buffer[rx_tail] = 'A';

  TEST_ASSERT_EQUAL_CHAR('\0', uart_read_char());
}

void rx_push_pop_returns_the_same_byte(void){
  dma_remaining = 10;
  rx_tail = 0;
  dma_rx_buffer[rx_tail] = 'F';
  TEST_ASSERT_EQUAL_CHAR('F', uart_read_char());
  TEST_ASSERT_EQUAL_INT(1, rx_tail);
  
  dma_rx_buffer[rx_tail] = 'A';
  TEST_ASSERT_EQUAL_CHAR('A', uart_read_char());
  TEST_ASSERT_EQUAL_INT(2, rx_tail);

  dma_rx_buffer[rx_tail] = '!';
  TEST_ASSERT_EQUAL_CHAR('!', uart_read_char());
  TEST_ASSERT_EQUAL_INT(3, rx_tail);
}

void buffer_full_reject_next_push(void){
  // Full if head and tail are the same 
  tx_tail = 10;
  tx_head = 9;

  uint16_t old_head = tx_head;

  uart_putc('A');

  TEST_ASSERT_EQUAL_INT(old_head, tx_head);
}

void wrap_around_index255_to_index0(void){
  tx_head = 127;
  tx_tail = 120;

  uart_putc('#');
  TEST_ASSERT_EQUAL_INT(0, tx_head);

  uart_putc('#');
  TEST_ASSERT_EQUAL_INT(1, tx_head);

  uart_putc('#');
  TEST_ASSERT_EQUAL_INT(2, tx_head);
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(rx_empty_buffer_pop);
  RUN_TEST(rx_push_pop_returns_the_same_byte);
  RUN_TEST(buffer_full_reject_next_push);
  RUN_TEST(wrap_around_index255_to_index0);
  return UNITY_END();
}

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
