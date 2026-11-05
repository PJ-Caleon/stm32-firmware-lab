#include "adc.h"
#include "potentiometer.h"

uint16_t Potentiometer_ReadRaw(void){
  uint16_t raw = ADC_GetScanPotRaw();
  return (uint16_t)(((uint32_t)raw * 3300U) / 4095U);
}
