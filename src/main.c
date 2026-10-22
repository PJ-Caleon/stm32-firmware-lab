#include <stdint.h>
#include "stm32f103.h"
#include "clock.h"
#include "stateMachine.h"
#include "port_pin.h"

// Global Variables 
volatile uint32_t last_time = 0;
volatile uint32_t interval = 0;
volatile uint8_t exti_flag = 0; // PINA0: 0 not pressed, 1 pressed 
volatile uint32_t interrupt_timestamp = 0;
static uint32_t last_valid_time = 0;

volatile uint32_t dwt_start = 0;
volatile uint32_t dwt_isr = 0;

/*
  Clear pending bit in ISR
  Record the time 
  Set a flag 
*/
// EXTI0 since that is where PA0 lives at 
void EXTI0_IRQHandler(void){
  dwt_isr = DWT_CYCCNT;
  EXTI->PR = (1 << 0); // Acknowledges interrupt occured 
  interrupt_timestamp = millis();
  exti_flag = 1;
}

/*
  Init DWT - counts how many cycles occured since startup
*/
void dwt_init(void){
  SET_BIT(DEMCR, TRCENA); // Enable trace enable 
  DWT_CYCCNT = 0; // Resets the cycle count 
  SET_BIT(DWT_CTRL, CYCCNTENA); // Enable Cycle Counter Enable 
}

int main(void){

  setClock();
  systick_init();
  dwt_init();

  // Enable AFIO Clock 
  SET_BIT(RCC->APB2ENR, (1 << 0));

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

  // EXTI# access pins A#, B#, C# & Setting it to Port A 
  // Tells STM, with EXTICR0 at Port A, set Pin A-0 to become the interrupt 
  MODIFY_REG(AFIO->EXTICR[0], (0xF << 0), (EXTI_PORTA << 0));

  SET_BIT(EXTI->IMR, (1 << 0));

  SET_BIT(EXTI->FTSR, (1 << 0));

  NVIC->ISER[0] = (1 << 6);

  dwt_start = DWT_CYCCNT;
  EXTI->SWIER = 1;

  // State Machine Var 
  StateMachine reaction_game = {0};
  reaction_game.current_state = IDLE;

  while(1){
    
    uint32_t current_time = millis();
    volatile uint8_t btn_event = 0;

    // Triggered if PA0 is pressed 
    if (exti_flag){
      exti_flag = 0;
      
      // Checks if 500 ms has passed since last press 
      if ((interrupt_timestamp - last_valid_time >= 500)){
        last_valid_time = interrupt_timestamp;

        btn_event = 1;
      }
    }

    stateMachine_update(&reaction_game, btn_event, current_time);
  }
}
