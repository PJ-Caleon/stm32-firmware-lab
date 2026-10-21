#include <stdint.h>
#include "stm32f103.h"
#include "clock.h"

#define PINC13 (1UL << 13)
#define PINA0 (1UL << 0)

// Global Variables 
volatile uint32_t last_time = 0;
volatile uint32_t interval = 0;
volatile uint32_t count = 0;
volatile uint8_t exti_flag = 0;
volatile uint32_t interrupt_timestamp = 0;

/*
  Clear pending bit in ISR
  Record the time 
  Set a flag 
*/
  
// EXTI0 since that is where PA0 lives at 
void EXTI0_IRQHandler(void){
  EXTI->PR = (1 << 0); // Acknowledges interrupt occured 
  interrupt_timestamp = millis();
  exti_flag = 1;
}

int main(void){

  setClock();
  systick_init();

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

  /*
    Set PINA0 to interrupt
    Unmask line 0 
    Select the falling edge 
    Enable IRQ 6 in the NVIC 
  */
  SET_BIT(AFIO->EXTICR[0], PINA0);

  SET_BIT(EXTI->IMR, (1 << 0));

  SET_BIT(EXTI->FTSR, (1 << 0));
  SET_BIT(EXTI->FTSR, (1 << 0));

  NVIC->ISER[0] = (1 << 6);

  while(1){
    
    if (exti_flag){
      exti_flag = 0;

      // Does something 
    }

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
        
        delay_ms(10);
      }
    }

    if (count == 0){
      interval = 1000;
    }
    if (count == 1){
      interval = 500;
    }
    if (count == 2){
      interval = 100;
    }

    if (millis() - last_time >= interval){
      last_time = millis();
      if (READ_BIT(GPIOC->ODR, PINC13)){
        GPIOC->BRR = PINC13;
      } else{
        GPIOC->BSRR = PINC13;
      }
    }
  }
}
