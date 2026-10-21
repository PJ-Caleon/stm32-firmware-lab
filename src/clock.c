#include <stdbool.h>

#include "stm32f103.h"
#include "clock.h"

#define RCC_CR_HSEON (1U << 16)

void setClock(void){
  // Turn on external crystal HSE 
  SET_BIT(RCC->CR, (RCC_CR_HSEON));
  
  // Wait for HSE cyystal to be stable
  while(!(READ_BIT(RCC->CR, RCC_CR_HSEON)));
}


