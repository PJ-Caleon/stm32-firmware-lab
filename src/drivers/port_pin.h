#ifndef PORT_PIN_H
#define PORT_PIN_H

/*
  port_pin - the ONE place that decides what every pin does.
  If a pin misbehaves, look here first.

    PC13  LED (active-low)              output 2 MHz push-pull
    PA0   Interrupt Button              input, pull-up
    PA9   USART1 TX                     alternate function push-pull 50 MHz
    PA10  USART1 RX                     floating input
*/

#define PINC13 (1UL << 13)
#define PINA1 (1UL << 1)
#define PINA2 (1UL << 2)
#define PINA9 (1UL << 9)
#define PINA10 (1UL << 10)

void port_pin_init(void);   // enables GPIO/AFIO clocks and configures every pin above
void led_on(void);
void led_off(void);

#endif
