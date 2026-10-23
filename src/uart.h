#ifndef UART_H
#define UART_H

void uart1_enable(void);
void init_tx1_rx1(void);
void enable_baud_rate(uint32_t pclk_freq);
void uart_putc(char c);
char uart_getc(void);
void uart_puts(const char *s);
int _write(int file, char *ptr, int len);

#endif
