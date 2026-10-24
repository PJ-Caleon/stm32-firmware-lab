#include <stdint.h>
#include "clock.h"
#include "port_pin.h"
#include "exti.h"
#include "uart.h"
#include "button.h"
#include "stateMachine.h"
#include "shell.h"

int main(void){

  // Core timing
  setClock();
  systick_init();
  dwt_init();

  // Hardware
  port_pin_init();        // every pin + GPIO/AFIO clocks
  uart_init(PCLK2_HZ);    // USART1 + its DMA channels
  exti1_init();           // PA1 button interrupt
  exti1_latency_test();   // optional: fills dwt_start / dwt_isr for the debugger

  // Application
  shell_init();

  // State Machine Var 
  StateMachine reaction_game = {0};
  reaction_game.current_state = IDLE;

  while(1){
    shell_poll();

    uint32_t current_time = millis();
    uint8_t btn_event = button_poll();

    stateMachine_update(&reaction_game, btn_event, current_time);
  }
}
