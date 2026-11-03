#include <stdint.h>
#include "stm32f103.h"
#include "dma.h"
#include "uart.h"
#include "ring_buffer.h"

#define UART_BAUD            115200
#define UART_RX_BUFFER_SIZE  256
#define TX_QUEUE_SIZE        128

#define UART1_TX_DMA_CH  4
#define UART1_RX_DMA_CH  5

__attribute__((weak)) volatile uint8_t dma_rx_buffer[UART_RX_BUFFER_SIZE];
volatile uint16_t rx_tail = 0;

static uint16_t rx_head(void){
  return UART_RX_BUFFER_SIZE - dma1_channel_remaining(UART1_RX_DMA_CH);
}

__attribute__((weak)) int uart_data_available(void){
  return (rx_tail != rx_head());
}

char uart_read_char(void){
  if (!uart_data_available()){
    return '\0';
  }
  char c = (char)dma_rx_buffer[rx_tail];
  rx_tail = (rx_tail + 1) % UART_RX_BUFFER_SIZE;
  return c;
}

static char tx_buffer_memory[TX_QUEUE_SIZE];
static ring_buffer_t tx_ring;

volatile uint8_t tx_busy = 0;
static volatile uint16_t tx_chunk_len = 0;

volatile uint16_t tx_head = 0;
volatile uint16_t tx_tail = 0;

// Weak stub wrappers for ring buffer operations to allow complete test-side overriding
__attribute__((weak)) uint8_t uart_stub_ring_queue(char c) {
  (void)c;
  return 1;
}

__attribute__((weak)) int uart_stub_ring_empty(void) {
  return 0;
}

static void uart_tx_kick(void){
  if (tx_busy){
    return;
  }

  if(uart_stub_ring_empty()){
    return;
  }

  tx_chunk_len = 1;
  tx_busy = 1;

  dma1_channel_config(UART1_TX_DMA_CH,
                      (uint32_t)(uintptr_t)&(UART1->DR),
                      (uint32_t)(uintptr_t)&tx_buffer_memory[tx_tail],
                      tx_chunk_len,
                      DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE);
  dma1_channel_enable(UART1_TX_DMA_CH);
}

void uart_tx_dma_done(void){
  tx_tail = (tx_tail + tx_chunk_len) % TX_QUEUE_SIZE;
  tx_busy = 0;
  uart_tx_kick();
}

int uart_try_putc(char c){
  if(!uart_stub_ring_queue(c)){
    return 0;
  }
  uart_tx_kick();
  return 1;
}

__attribute__((weak)) void uart_tx_wait_hook(void){}

void uart_putc(char c){
  while (!uart_try_putc(c)){
    uart_tx_kick();
    uart_tx_wait_hook();
  }
}

void uart_puts(const char *s){
  while (*s != '\0'){
    uart_putc(*s);
    s++;
  }
}

void uart_init(uint32_t pclk_freq){
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
  (void)RCC->APB2ENR;
  dma1_init();

  ring_buffer_init(&tx_ring, tx_buffer_memory, TX_QUEUE_SIZE);
  tx_head = 0;
  tx_tail = 0;

  dma1_channel_config(UART1_RX_DMA_CH,
                      (uint32_t)(uintptr_t)&(UART1->DR),
                      (uint32_t)(uintptr_t)dma_rx_buffer,
                      UART_RX_BUFFER_SIZE,
                      DMA_CCR_MINC | DMA_CCR_CIRC);
  dma1_channel_enable(UART1_RX_DMA_CH);

  dma1_set_tc_callback(UART1_TX_DMA_CH, uart_tx_dma_done);

  UART1->BRR = pclk_freq / UART_BAUD;
  UART1->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
  UART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}
