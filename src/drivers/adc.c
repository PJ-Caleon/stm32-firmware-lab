#include <stdint.h>
#include "stm32f103.h"
#include "adc.h"
#include "clock.h"
#include "dma.h"
#include "temperature.h"

/*
  Set ADC prescaler to divide by 6 (12 MHz)
  Enable ADC1 clock 
  Set ADON 
  wait then run calibration (CAL bit, wait for it to clear)
*/

/*
  Scan mode with DMA 
  Convert channel 2 and temp sensor continuously 
  into 2 element array
*/

#define ADC_DMA_BUFFER_SIZE 2

static volatile uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];

void ADC_init(void){
  // Enable ADC and GPIO Clock
  SET_BIT(RCC->APB2ENR, (RCC_APB2ENR_IOPAEN)); // Enable GPIOA clock  
  SET_BIT(RCC->APB2ENR, (RCC_APB2ENR_ADC1EN)); // Enable ADC1 clock
  dma1_init();

  // Set Prescaler of 6 
  MODIFY_REG(RCC->CFGR, (ADC_PRESCALER_MASK), (ADC_PRESCALER_DIV6));

  // Configure ADC control register for Scan + continuous + dma 
  SET_BIT(ADC1->CR1, (ADC_CR1_SCAN));
  SET_BIT(ADC1->CR2, ADC_CR2_CONT | ADC_CR2_DMA | (1U << 20) | (7U << 17));

  // Configure sequence length 
  MODIFY_REG(ADC1->SQR1, (0xFU << 20), (1U << 20));

  // Configure sequence channels: SQ1 = pot (CH2), SQ2 = Temp (CH16) 
  MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ1_MASK, (2U << ADC_SQR3_SQ1_POS));
  MODIFY_REG(ADC1->SQR3, ADC_SQR3_SQ2_MASK, (16U << ADC_SQR3_SQ2_POS));

  // Sample time to 239.5 cycles 
  MODIFY_REG(ADC1->SMPR2, (7U << 6), (7U << 6));    // Channel 2
  MODIFY_REG(ADC1->SMPR1, (7U << 18), (7U << 18));  // Channel 16

  // Enable internal temperature sensor & vref channel 
  SET_BIT(ADC1->CR2, (ADC_CR2_TSVREFE));

  // Enable ADC (ADON) and Calibration 
  SET_BIT(ADC1->CR2, (ADC_CR2_ADON));
  delay_ms(0.1);

  SET_BIT(ADC1->CR2, (ADC_CR2_RSTCAL));
  while (READ_BIT(ADC1->CR2, ADC_CR2_RSTCAL) != 0U){}

  SET_BIT(ADC1->CR2, (ADC_CR2_CAL));
  while (READ_BIT(ADC1->CR2, ADC_CR2_CAL) != 0U) {}

  // Configure DMA1 channel 1 for ADC1 
  dma1_channel_config(1,
                      (uint32_t)&(ADC1->DR),
                      (uint32_t)adc_dma_buffer,
                      ADC_DMA_BUFFER_SIZE,
                      DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_PSIZE_16 | DMA_CCR_MSIZE_16);
  // MINC - allows address to increment when new data is added 
  // CIRC - enables circular mode for continuous data stream 
  // PSIZE - Size of the peripheral transfer 
  // MSIZE - Size of the memory data

  dma1_channel_enable(1);

  // Start continuous scan loop 
  SET_BIT(ADC1->CR2, (ADC_CR2_SWSTART));
}

// DMA Buffer Getters
__attribute__((weak)) uint16_t ADC_GetScanPotRaw(void) {
  return adc_dma_buffer[0];
}

uint16_t ADC_GetScanTempRaw(void) {
  return adc_dma_buffer[1];
}
