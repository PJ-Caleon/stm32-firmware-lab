#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void ADC_init(void);
uint16_t ADC_GetScanPotRaw(void);
uint16_t ADC_GetScanTempRaw(void);

#endif
