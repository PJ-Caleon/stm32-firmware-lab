#ifndef UART_H
#define UART_H

#include <stdint.h>

/*
  uart - USART1 driver. Owns the pins' peripheral side, both buffers, and
  how DMA is used for RX and TX. Nothing outside this file touches UART1->...

    RX: DMA channel 5 fills a 256-byte circular buffer on its own.
        The CPU just reads behind the DMA write position (no RX interrupt).
    TX: uart_putc()/uart_puts() put bytes in a 128-byte queue; DMA channel 4
        drains it in the background.

  Pin setup (PA9/PA10) is in port_pin.c, DMA channel mechanics are in dma.c.
*/

void uart_init(uint32_t pclk_freq);   // call AFTER port_pin_init()

void uart_putc(char c);               // queue one byte (waits only if the queue is full)
void uart_puts(const char *s);

int uart_data_available(void);        // 1 if at least one received byte is waiting
char uart_read_char(void);            // next received byte, '\0' if none

#endif
