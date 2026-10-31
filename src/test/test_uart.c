#include <string.h>
#include <unity.h>
#include "uart.h"

static uint32_t dma_remaining = 256;
extern volatile int8_t dma_rx_buffer[256];
extern volatile uint16_t rx_tail;
extern volatile uint16_t tx_head;
extern volatile uint16_t tx_tail;

extern void uart_tx_dma_done(void);
extern int uart_try_putc(char c);

static int tx_pending = 0;
static uint16_t tx_pending_len = 0;
static uint32_t tx_total_send = 0;

static void finish_pending_dma(void){
  if(tx_pending){
    tx_pending = 0;
    tx_total_send += tx_pending_len;
    uart_tx_dma_done();
  }
}

void uart_tx_wait_hook(void){
  finish_pending_dma();
}

void setUp(){
  while(tx_pending){
    finish_pending_dma();
  }
  tx_total_send = 0;
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

  TEST_ASSERT_EQUAL_INT(0, uart_try_putc('A'));
  TEST_ASSERT_EQUAL_INT(old_head, tx_head);
}

void wrap_around_index127_to_index0(void){
  tx_head = 127;
  tx_tail = 120;

  uart_putc('#');
  TEST_ASSERT_EQUAL_INT(0, tx_head);

  uart_putc('#');
  TEST_ASSERT_EQUAL_INT(1, tx_head);

  uart_putc('#');
  TEST_ASSERT_EQUAL_INT(2, tx_head);
}

void long_string_is_not_truncated(void){
  char msg[301];
  memset(msg, 'x', 300);
  msg[300] = '\0';

  uart_puts(msg);

  while(tx_pending){
    finish_pending_dma();
  }
  TEST_ASSERT_EQUAL_INT32(300, tx_total_send);
}

int main(void){
  UNITY_BEGIN();
  RUN_TEST(rx_empty_buffer_pop);
  RUN_TEST(rx_push_pop_returns_the_same_byte);
  RUN_TEST(buffer_full_reject_next_push);
  RUN_TEST(wrap_around_index127_to_index0);
  RUN_TEST(long_string_is_not_truncated);
  return UNITY_END();
}

void dma1_init(void){}

void dma1_channel_config(uint8_t channel, uint32_t periph_addr, uint32_t mem_addr, uint16_t size, uint32_t ccr) {
    (void)periph_addr; (void)mem_addr; (void)ccr;
    if (channel == 4){          // TX channel
      tx_pending_len = size;
    }
}
void dma1_channel_enable(uint8_t channel) {
    if (channel == 4){
      tx_pending = 1;
    }
}

void dma1_set_tc_callback(uint8_t channel, void (*callback)(void)) { (void)channel; (void)callback; }
uint16_t dma1_channel_remaining(uint8_t channel) {
    (void)channel;
    return dma_remaining;
}
