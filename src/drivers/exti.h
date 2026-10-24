#ifndef EXTI_H
#define EXTI_H

#include <stdint.h>

/*
  exti - external (pin) interrupts. This driver only reports that an edge
  happened (flag + timestamp). Deciding what an edge MEANS (debounce) is
  button.c's job.

  To use a pin as an interrupt, call exti_enable_line() once at startup.
  The port constants (EXTI_PORTA, EXTI_PORTB, EXTI_PORTC) are in stm32f103.h.
*/

// Which pin transition raises the interrupt
typedef enum {
  EXTI_FALLING,   // high -> low (button to ground with a pull-up)
  EXTI_RISING,    // low -> high
  EXTI_BOTH       // either
} exti_edge_t;

/*
  Make pin number 'line' of 'port' raise an interrupt.
  Pin number == line number: PA3, PB3 and PC3 are all "line 3".
  Example: exti_enable_line(EXTI_PORTA, 0, EXTI_FALLING);   // PA0
  Does NOT configure the pin itself (input/pull-up): that is port_pin.c.
*/
void exti_enable_line(uint8_t port, uint8_t line, exti_edge_t edge);

extern volatile uint8_t exti_flag;              // set by the ISR, cleared by the consumer
extern volatile uint32_t interrupt_timestamp;   // millis() at the moment of the edge

extern volatile uint32_t dwt_start;             // debug: cycle count when the test edge was forced
extern volatile uint32_t dwt_isr;               // debug: cycle count on ISR entry (latency = isr - start)

void exti0_init(void);           // the PA0 button: falling edge
void exti0_latency_test(void);   // forces one edge and swallows it; read dwt_start/dwt_isr in the debugger

void exti1_init(void);           // the PA1 button: falling edge
void exti1_latency_test(void);   // forces one edge and swallows it; read dwt_start/dwt_isr in the debugger
#endif
