#include <stdint.h>
#include "stm32f103.h"

#define PINC13 (1UL << 13)
#define PINA0 (1UL << 0)

int main(void){
  /*
    Enable GPIOC clock with APB2ENR bit 4
    IOPBEN is bit 3, IOPCEN is bit 4

    -> is a shorthand for pointers to structs, its like the . but for pointers 
  */
  SET_BIT(RCC->APB2ENR, (1 << 4));

  /*
    Configure PC13 as 2 MHz push-pull output 
    Pin 13 uses bits 23:20 in CRH (20 is the start bit for pin 13)
  */
  CLEAR_BIT(GPIOC->CRH, (0xF << 20));
  SET_BIT(GPIOC->CRH, (0x2 << 20)); // Sets to 0b0010 (output max 2 MHz)

  /*
    Enable GPIOA clock with APB2ENR bit 4
    (enable/disable << port num)
  */
  SET_BIT(RCC->APB2ENR, (1 << 2));

  /*
    Configure PA0 as input with internall pull-up
    CRL:CNF=10, MODE=00

    Clear 4 bit for pin 0 (bits 3:0 in CRL)
  */
  CLEAR_BIT(GPIOA->CRL, (0xF << 0));
  
  /*
    Configure CRL bits 3:0 to 1000 (0x8), CNF=10,MODE=00
  */
  MODIFY_REG(GPIOA->CRL, (0xF << 0), (0x8 << 0));

  /*
    Set the ODR bit to 1 to be a pull-up to make the pin be 10001  
  */
  SET_BIT(GPIOA->ODR, PINA0);


  // Global Variables 

  volatile uint32_t count = 0;
  volatile uint32_t timer = 0;

  while(1){
  
    /*
      Read PINA0 button press 
      press is considered if the press is done within 10ms
      Any longer and it is not a valid press 
    */
    if(!(READ_BIT(GPIOA->IDR, PINA0))){
      while(!(READ_BIT(GPIOA->IDR, PINA0))){
        
        if (count != 2){
          count++;
        }else{
          count = 0;
        } 

        for (volatile int i = 0; i < 100000; i++);
      }
    }

    if (count == 0){
      timer = 100000;
    }
    if (count == 1){
      timer = 50000;
    }
    if (count == 2){
      timer = 10000;
    }

    if (READ_BIT(GPIOC->ODR, PINC13)){
      GPIOC->BRR = PINC13;
    } else{
      GPIOC->BSRR = PINC13;
    }
    for (volatile int i = 0; i < timer; i++);
  }
}
