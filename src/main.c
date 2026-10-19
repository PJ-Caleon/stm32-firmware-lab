#include <stdint.h>
#include "stm32f103.h"

#define PIN13 (1UL << 13)

int main(void){
  /*
    Enable GPIOC clock with APB2ENR bit 4
    IOPBEN is bit 3, IOPCEN is bit 4

    -> is a shorthand for pointers to structs, its like the . but for pointers 
  */
  SET_BIT(RCC->APB2ENR, (1 << 4));

  /*
    Configure PC13 as 2 MHz push-pull output 
    Pint 13 uses bits 23:20 in CRH (20 is the start bit for pin 13)
  */
  CLEAR_BIT(GPIOC->CRH, (0xF << 20));
  SET_BIT(GPIOC->CRH, (0x2 << 20)); // Sets to 0b0010 (output max 2 MHz)

  while(1){
    /*
      Toggle the BSRR/BRR 
      The STM LED is active low (0 = on, 1 = off)
    */

    // On: write to BRR to reset pin low 
    GPIOC->BRR = PIN13;
    for (int i = 0; i < 500000; i++);

    // Off: write to BSRR to set pin high 
    GPIOC->BSRR = PIN13;
    for (int i = 0; i < 500000; i++);
  }
}
