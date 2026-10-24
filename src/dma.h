#ifndef DMA_H
#define DMA_H

#define DMA_BUFFER_SIZE 256
#define DMA_CCR_EN (1U << 0)
#define DMA_CCR_CIRC (1U << 5)
#define DMA_CCR_MINC (1U << 7)
#define DMA_CCR_PL_0 (1U << 12)
#define RCC_AHBENR_DMA1EN (1U << 0)
#define UART_CR3_DMAR (1U << 6)
#define DMA_CCR_TCIE (1U << 1)
#define DMA_CCR_DIR (1U << 4)
#define USART_CR3_DMAT (1U << 7)

void dma1_enable(void);

#endif
