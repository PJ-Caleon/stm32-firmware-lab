#include <stdbool.h>

#include "stm32f103.h"
#include "clock.h"

#define RCC_CR_HSEON (1U << 16)
#define FLASH_ACR_PRFTBE (1U << 4) 
#define FLASH_ACR_LATENCY (0x7)
#define FLASH_ACR_LATENCY_2 (0x2) // 48 - 72 MHz 
#define FLASH_ACR_LATENCY_1 (0x1) // 24 - 48 MHz 
#define FLASH_ACR_LATENCY_0 (0x0) // 0 - 24 MHz 

void setClock(void){
  // Turn on external crystal HSE 
  SET_BIT(RCC->CR, (RCC_CR_HSEON));
  
  // Wait for HSE cyystal to be stable
  while(!(READ_BIT(RCC->CR, RCC_CR_HSEON)));

  // enable prefetch buffer (FLASH ACR)
  SET_BIT(FLASH->ACR, (FLASH_ACR_PRFTBE)); 

  // Flash 2 wait state 
  CLEAR_BIT(FLASH->ACR, (FLASH_ACR_LATENCY));
  SET_BIT(FLASH->ACR, (FLASH_ACR_LATENCY_2));
}


