#ifndef UART_BOOT_H
#define UART_BOOT_H

#include <stdint.h>

void uart_boot_init(uint32_t pclk_freq);
void uart_boot_putc(uint8_t byte);
int uart_boot_getc(uint8_t *byte, uint32_t timeout_ms);

int uart_boot_read_line(char *buf, uint32_t max_len, uint32_t timeout_ms);
void uart_boot_protocol_loop(void);

#endif
