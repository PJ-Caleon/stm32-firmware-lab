#include "stm32f103.h"
#include "temperature.h"
#include "adc.h"
#include "clock.h"

void temperature_init(void){
  // Enable internal temperature and VREF channel 
  SET_BIT(ADC1->CR2, ADC_CR2_TSVREFE);
  delay_ms(1);

  // Set sample time for channel 16 to 239.5 cycles 
  MODIFY_REG(ADC1->SMPR1, (7U << 18), (7U << 18));
  MODIFY_REG(ADC1->SMPR1, (7U << 21), (7U << 21));
}

int32_t temperature_ReadCelsius(void){
  uint16_t raw = ADC_ReadChannel(16);

  int32_t v_mv = (int32_t)(((uint32_t)raw * 3300UL) / 4095UL);
  int32_t temp = ((1430 - v_mv) * 1000) / 4300 + 25;

  return temp;
}

uint16_t internalVref_ReadRaw(void){
  return ADC_GetScanTempRaw();
}
