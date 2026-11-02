#ifndef PORT_PIN_H
#define PORT_PIN_H

/*
  port_pin - the ONE place that decides what every pin does.
  If a pin misbehaves, look here first.
*/

#define PINC13 (1UL << 13)
#define PINA1  (1UL << 1)
#define PINA2  (1UL << 2)
#define PINA9  (1UL << 9)
#define PINA10 (1UL << 10)

#define PINA4  (1UL << 4) // CS
#define PINA5  (1UL << 5) // SCK 
#define PINA6  (1UL << 6) // MISO 
#define PINA7  (1UL << 7) // MOSI 

void port_pin_init(void);   // enables GPIO/AFIO clocks and configures every pin above
void led_on(void);
void led_off(void);

#endif
