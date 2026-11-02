#include <stdbool.h>
#include "stm32f103.h"
#include "clock.h"
#include "FreeRTOS.h"
#include "task.h"

// getter Function 
__attribute__((weak)) uint32_t get_ms_tick(void){
  return (uint32_t)xTaskGetTickCount();
}

__attribute__((weak)) void delay_ms(uint32_t ms){
  vTaskDelay(pdMS_TO_TICKS(ms));
}

__attribute__((weak)) uint32_t millis(void){
  return (uint32_t)xTaskGetTickCount();
}

__attribute__ ((weak)) void systick_init(void){}

/*
  Init DWT - counts how many cycles occured since startup
*/
void dwt_init(void){
  SET_BIT(DEMCR, TRCENA); // Enable trace enable 
  DWT_CYCCNT = 0; // Resets the cycle count 
  SET_BIT(DWT_CTRL, CYCCNTENA); // Enable Cycle Counter Enable 
}

void setClock(void){
  // Turn on external crystal HSE 
  SET_BIT(RCC->CR, (RCC_CR_HSEON));
  
  // Wait for HSE crystal to be stable (HSERDY, not HSEON: HSEON is just the
  // bit we wrote, it is set immediately and says nothing about the crystal)
  while(!(READ_BIT(RCC->CR, RCC_CR_HSERDY)));

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
