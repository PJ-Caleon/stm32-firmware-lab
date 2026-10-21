#include <stdbool.h>
#include "stm32f103.h"
#include "clock.h"

volatile uint32_t ms_tick = 0;

void delay_ms(uint32_t ms){
  uint32_t start_tick = ms_tick;

  while((ms_tick - start_tick) < ms);
}

uint32_t millis(void){
  return ms_tick;
}

void systick_init(void){
  // Set reload value for 1ms tick (72,000 cycles)
  SYSTICK->LOAD = 72000 - 1;

  // Clear current counter value 
  SYSTICK->VAL = 0;

  // Enable systick, interupt, set clksrc to processor clock 
  SYSTICK->CTRL = SYSTICK_EN | SYSTICK_TICKINT | SYSTICK_CLKSRC;
}

void SysTick_Handler(void){
  ms_tick++;
}

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
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLXTPRE));
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_PLLMULL));

  SET_BIT(RCC->CFGR, (RCC_CFGR_PLLSRC)); // PLL Source
  SET_BIT(RCC->CFGR, (RCC_CFGR_PLLMULL9)); // PLL Multiplier 9
  SET_BIT(RCC->CFGR, (RCC_CFGR_PPRE1_DIV2)); // APB1 divided by 2

  // Turn on PLL 
  SET_BIT(RCC->CR, (RCC_CR_PLLON));
  while (!(READ_BIT(RCC->CR, (RCC_CR_PLLRDY))));
  
  // Set PLL as sysclk
  CLEAR_BIT(RCC->CFGR, (RCC_CFGR_SW));
  SET_BIT(RCC->CFGR, (RCC_CFGR_SW_PLL));
  while(!(READ_BIT(RCC->CFGR, (RCC_CFGR_SWS_PLL))));
}
