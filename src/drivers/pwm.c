#include "stm32f103.h"
#include "pwm.h"
#include "adc.h"

void pwm_init(void){
  // Enable TIM3 clock  
  SET_BIT(RCC->APB1ENR, (RCC_APB1ENR_TIM3EN));

  // Set Prescaler to 71 (72 MHz/ 72)
  TIM3->PSC = 71;

  // Set Auto reload to 999 (1 MHz/ 1000)
  TIM3->ARR = 999;

  // Configure CCMR1 for PWM Mode 1 on channel 1 
  MODIFY_REG(TIM3->CCMR1, (0xFF << 0), (6U << 4) | (1U << 3));

  // Enable channel 1 output 
  SET_BIT(TIM3->CCER, (1U << 0));

  // Enable timer counter 
  SET_BIT(TIM3->CR1, (1U << 0));

  // Start duty cycle 
  TIM3->CCR1 = 500;
}

uint16_t pwm_update(void){
  uint16_t raw = ADC_GetScanPotRaw();
  uint32_t duty = ((uint32_t)raw * 1000UL) / 4095UL;
  TIM3->CCR1 = (uint16_t)duty;
  return (uint16_t)duty;
}
