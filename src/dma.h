#ifndef DMA_H
#define DMA_H

#define BUFFER_SIZE 256
#define DMA_CCR_EN (1U << 0)
#define DMA_CCR_CIRC (1U << 5)
#define DMA_CCR_MINC (1U << 7)
#define DMA_CCR_PL_0 (1U << 12)
#define RCC_AHBENR_DMA1EN (1U << 0)
#define UART_CR3_DMAR (1U << 6)

void dma1_enable(void);

#endif
