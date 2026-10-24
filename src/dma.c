#include <stdint.h>
#include "dma.h"
#include "stm32f103.h"

// Initialize DMA1 Clock 
void dma1_enable(void){
  SET_BIT(RCC->AHBENR, (1 << 0));
}

