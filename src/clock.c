#include <stdbool.h>

#include "stm32f103.h"
#include "clock.h"

#define RCC_CR_HSEON (1U << 16)

#define RCC_CFGR_PLLSRC (1U << 16)
#define RCC_CFGR_PLLXTRE (1U << 17)
#define RCC_CFGR_PLLMULL (0xF << 18)
#define RCC_CFGR_PLLMULL9 (7U << 18)
#define RCC_CFGR_PPRE1_DIV2 (4U << 8)
#define RCC_CR_PILLON (1U << 24)
#define RCC_CR_PLLRDY (1U << 25)

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

  // Configure RCC and PLL reset  
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLSRC));
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLXTRE));
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLMULL));

  SET_BIT(RCC->CFGR, (RCC_CFGR_PLLSRC)); // PLL Source
  SET_BIT(RCC->CFGR, (RCC_CFGR_PLLMULL9)); // PLL Multiplier 9
  SET_BIT(RCC->CFGR, (RCC_CFGR_PPRE1_DIV2)); // APB1 divided by 2

  // Turn on PLL 
  SET_BIT(RCC->CR, (RCC_CR_PILLON));
  while (!(READ_BIT(RCC-CR, (RCC_CR_PLLRDY))));
}


