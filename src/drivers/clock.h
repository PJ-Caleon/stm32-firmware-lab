#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

#define SYSCLK_HZ 72000000UL
#define PCLK2_HZ  SYSCLK_HZ     // APB2 (USART1, GPIO, AFIO) is not divided

extern volatile uint32_t ms_tick;

void setClock(void);
void systick_init(void);
void dwt_init(void);
void delay_ms(uint32_t ms);
uint32_t millis(void);

#endif
