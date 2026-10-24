#ifndef UART_H
#define UART_H

#include <stdint.h>

#define TX_QUEUE_SIZE 128

void uart1_enable(void);
void init_tx1_rx1(void);
void enable_baud_rate(uint32_t pclk_freq);
void uart_putc(char c);
char uart_getc(void);
void uart_puts(const char *s);
int _write(int file, char *ptr, int len);
void uart_rx_interrupt_init(void);
int uart_data_available(void);
char uart_read_char(void);
void USART_IRQHandler(void);
void process_uart_dma_buffer(void);

typedef struct Tx_queue{
  uint8_t buffer[TX_QUEUE_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
  volatile uint8_t busy;
} tx_queue;

extern tx_queue tx_q;
extern volatile uint8_t burst_ready;

#endif
