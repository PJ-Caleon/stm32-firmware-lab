#include <stdbool.h>

#include "stm32f103.h"
#include "clock.h"

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

  // Configure RCC and PLL reset  
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLSRC));
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLXTRE));
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLMULL));

  SET_BIT(RCC->CFGR, (RCC_CFGR_PLLSRC)); // PLL Source
  SET_BIT(RCC->CFGR, (RCC_CFGR_PLLMULL9)); // PLL Multiplier 9
  SET_BIT(RCC->CFGR, (RCC_CFGR_PPRE1_DIV2)); // APB1 divided by 2

  // Turn on PLL 
  SET_BIT(RCC->CR, (RCC_CR_PILLON));
  while (!(READ_BIT(RCC->CR, (RCC_CR_PLLRDY))));
  
  // Set PLL as sysclk
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_SW));
  SET_BIT(RCC->CFGR, (RCC_CFGR_SW_PLL));
  while(!(READ_BIT(RCC->CFGR, (RCC_CFGR_SW_PLL))));
}
