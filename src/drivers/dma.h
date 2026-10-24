#ifndef DMA_H
#define DMA_H

#include <stdint.h>

/*
  dma - generic DMA1 channel driver.
  It knows NOTHING about UART or any buffer. A peripheral driver (uart.c)
  tells it which addresses to move and gets a callback when a transfer ends.

  Channel numbers are 1..7, same as the reference manual (DMA1_Channel1..7).
*/

typedef void (*dma_callback_t)(void);

void dma1_init(void);   // enable DMA1 clock

// Set up a channel. ccr_flags: DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_TCIE ...
// The channel is left DISABLED; call dma1_channel_enable() to start it.
void dma1_channel_config(uint8_t ch, uint32_t periph_addr, uint32_t mem_addr,
                         uint16_t count, uint32_t ccr_flags);
void dma1_channel_enable(uint8_t ch);
void dma1_channel_disable(uint8_t ch);
uint16_t dma1_channel_remaining(uint8_t ch);   // CNDTR: transfers still to go

// Run cb (in interrupt context) when the channel finishes a transfer.
// Also enables the channel's NVIC interrupt. The channel needs DMA_CCR_TCIE set.
void dma1_set_tc_callback(uint8_t ch, dma_callback_t cb);

#endif
