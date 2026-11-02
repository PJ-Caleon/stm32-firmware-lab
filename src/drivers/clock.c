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
  if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
    // 72 MHz clock = 72,000 cycles per millisecond
    uint32_t cycles = (uint32_t)(ms * 72000.0f);
    uint32_t start = DWT_CYCCNT;
    while ((DWT_CYCCNT - start) < cycles);
    return;
  }
  TickType_t ticks = pdMS_TO_TICKS(ms);
  if (ticks == 0) {
    ticks = 1; // Ensure at least 1 tick delay if non-zero 
  }
  vTaskDelay(ticks);
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
  // 1. Enable High Speed External (HSE) clock and wait for it to stabilize
  SET_BIT(RCC->CR, RCC_CR_HSEON);
  while (!(READ_BIT(RCC->CR, RCC_CR_HSERDY)));

  // 2. Configure Flash Access Control Register (ACR) for 72 MHz operation
  // Enable Prefetch Buffer and set 2 wait states (LATENCY = 2)
  SET_BIT(FLASH->ACR, FLASH_ACR_PRFTBE);
  MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_2);

  // 3. Configure bus prescalers 
  // APB1 max is 36MHz, so divide AHB clock by 2 (PPRE1 = /2)
  MODIFY_REG(RCC->CFGR, (7U << 8), RCC_CFGR_PPRE1_DIV2);

  // 4. Configure PLL Source and Multiplication Factor
  // Source = HSE, Multiplication = x9 (8MHz * 9 = 72MHz)
  MODIFY_REG(RCC->CFGR, 
             RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL, 
             RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);

  // 5. Enable PLL and wait until it is ready
  SET_BIT(RCC->CR, RCC_CR_PLLON);
  while (!(READ_BIT(RCC->CR, RCC_CR_PLLRDY)));

  // 6. Select PLL as the System Clock Source (SW = PLL) and wait for switch status (SWS)
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
  while ((RCC->CFGR & (3U << 2)) != RCC_CFGR_SWS_PLL);
}
