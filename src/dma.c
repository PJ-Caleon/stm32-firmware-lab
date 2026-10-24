#include <stdint.h>
#include "dma.h"
#include "stm32f103.h"
#include "uart.h"

/*
  Configure DMA1 Channel 5 to copy incoming bytes 
  from UART1 Data Register into RAM buffer 
*/
uint8_t dma_rx_buffer[DMA_BUFFER_SIZE];

void dma1_enable(void){
  // Enable DMA1 clock in RCC 
  SET_BIT(RCC->AHBENR, (RCC_AHBENR_DMA1EN));

  // Disable DMA1 channel 5 before configuring 
  // CH[4] corresponds to channel 5
  CLEAR_BIT(DMA1->CH[4].CCR, (DMA_CCR_EN));

  // Set peripheral address to UART1 Data Register 
  DMA1->CH[4].CPAR = (uint32_t)&(UART1->DR);

  // Set memory address to 256 byte buffer 
  DMA1->CH[4].CMAR = (uint32_t)dma_rx_buffer;

  // Set transfer count 
  DMA1->CH[4].CNDTR = DMA_BUFFER_SIZE;

  // Configure CCR: Enable memory to incremement and circular mode 
  DMA1->CH[4].CCR = DMA_CCR_MINC | DMA_CCR_CIRC;

  // Enable DMA1 Channel 5
  SET_BIT(DMA1->CH[4].CCR, (DMA_CCR_EN));

  // Set DMAR bit in UART1 control register 3 
  // to enable DMA receiver request 
  SET_BIT(UART1->CR3, (UART_CR3_DMAR));
}
