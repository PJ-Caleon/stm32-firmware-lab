#include <stdint.h>
#include "stm32f103.h"
#include "adc.h"
#include "clock.h"

/*
  Set ADC prescaler to divide by 6 (12 MHz)
  Enable ADC1 clock 
  Set ADON 
  wait then run calibration (CAL bit, wait for it to clear)
*/
void ADC_Init(void){
  // Enable ADC and GPIO Clock
  SET_BIT(RCC->APB2ENR, (RCC_APB2ENR_IOPAEN)); // Enable GPIOA clock  
  SET_BIT(RCC->APB2ENR, (RCC_APB2ENR_ADC1EN)); // Enable ADC1 clock

  // Set Prescaler of 6 
  MODIFY_REG(RCC->CFGR, (ADC_PRESCALER_MASK), (ADC_PRESCALER_DIV6));

  // Enable ADC (ADON); 30th bit of the CR2 is the SWSTART
  SET_BIT(ADC1->CR2, (ADC_CR2_ADON));
  delay_ms(0.1);

  SET_BIT(ADC1->CR2, (ADC_CR2_RSTCAL));
  while (READ_BIT(ADC1->CR2, ADC_CR2_RSTCAL) != 0U){}

  SET_BIT(ADC1->CR2, ADC_CR2_CAL);
  while (READ_BIT(ADC1->CR2, ADC_CR2_CAL) != 0u) {}
}

