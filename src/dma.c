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

// Initialize DMA1 Channel 4 for UART1 TX 
void dma1_tx_init(void){
  SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
  SET_BIT(UART1->CR3, USART_CR3_DMAT);
}

// Startup for the DMA Transfer 
static void start_dma_tx(const uint8_t *data, uint16_t size){
  // Disable channel before config 
  CLEAR_BIT(DMA1->CH[3].CCR, DMA_CCR_EN);

  // Set peripheral address to UART1 data register 
  DMA1->CH[3].CPAR = (uint32_t)&(UART1->DR);

  // Set memory address 
  DMA1->CH[3].CMAR = (uint32_t)data;

  // Set transfer count  
  DMA1->CH[3].CNDTR = size;
  
  // Config CCR 
  DMA1->CH[3].CCR = DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE;

  // Enable NVIC IRQ for DMA1 channel 4 
  NVIC->ISER[0] = (1 << 14);
  tx_q.busy = 1;

  // Enable DMA channel 
  SET_BIT(DMA1->CH[3].CCR, DMA_CCR_EN);
}

void uart_dma_puts(const char *s) {
  while (*s) {
    uint16_t next_head = (tx_q.head + 1) % TX_QUEUE_SIZE;
    while (next_head == tx_q.tail); // Block if queue is full

    tx_q.buffer[tx_q.head] = (uint8_t)*s;
    tx_q.head = next_head;
    s++;
  }

  // If DMA is idle, start transfer immediately
  if (!tx_q.busy && (tx_q.head != tx_q.tail)) {
    tx_q.busy = 1;
        
    // For a ring buffer, calculate contiguous bytes up to the end or head
    uint16_t start_idx = tx_q.tail;
    uint16_t len;
        
    if (tx_q.head > tx_q.tail) {
      len = tx_q.head - tx_q.tail;
      tx_q.tail = tx_q.head; // Advance tail to head
    } else {
      len = TX_QUEUE_SIZE - tx_q.tail; // Send up to end of buffer first
      tx_q.tail = 0; // Wrap around
    }
      start_dma_tx(&tx_q.buffer[start_idx], len);
  }
}

void DMA1_Channel4_IRQHandler(void){
  if (DMA1->ISR & (1 << 13)){
    DMA1->IFCR = (1 << 13);
    if (tx_q.head != tx_q.tail){
      uint16_t start_idx = tx_q.tail;
      uint16_t len;

      if (tx_q.head > tx_q.tail){
        len = tx_q.head - tx_q.tail;
        tx_q.tail = tx_q.head;
      } else{
        len = TX_QUEUE_SIZE - tx_q.tail;
        tx_q.tail = 0;
      }

      start_dma_tx(&tx_q.buffer[start_idx], len);
    } else{
      tx_q.busy = 0;
    }
  }
}
