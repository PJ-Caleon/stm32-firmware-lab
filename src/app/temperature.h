#ifndef TEMPERATUE_H
#define TEMPERATUE_H

#include <stdint.h>

void temperature_init(void);
int32_t temperature_ReadCelsius(void);
uint16_t internalVref_ReadRaw(void);

#endif
