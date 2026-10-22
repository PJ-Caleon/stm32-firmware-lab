#include <stdlib.h>
#include <stdio.h>
#include "stateMachine.h"
#include "stm32f103.h"
#include "clock.h"
#include "port_pin.h"

static uint32_t lcg_seed = 1;

static uint32_t pseudo_random(void){
  lcg_seed = lcg_seed * 1103515245 + 12345;
  return lcg_seed;
}

void stateMachine_update(StateMachine *stateMachine, uint8_t button_pressed, uint32_t current_time){
  switch (stateMachine->current_state){
    case IDLE:
      // Led Off 
      GPIOC->BSRR = PINC13;
      
      if (button_pressed){
        // sets wait target in between 1-3 seconds 
        stateMachine->random_wait_target = current_time + (pseudo_random() % 2000 + 1000);
        stateMachine->current_state = WAIT_RANDOM;
      } break;
    
    case WAIT_RANDOM:
      // wait for 1-3 seconds 
      if (current_time >= stateMachine->random_wait_target){

        // Turn Led On
        GPIOC->BRR = PINC13;

        // Start Timer 
        stateMachine->timer_start_tick = current_time;
        stateMachine->current_state = GO;
      } break;

    case GO:
      if (button_pressed){
        // Get reaction_time
        stateMachine->reaction_time = current_time - stateMachine->timer_start_tick;

        if (stateMachine->reaction_time >= 1000){
          stateMachine->current_state = RESULT;
        } else{
          stateMachine->current_state = TOO_EARLY;
        }
      } break;

    case RESULT:
      for (int i = 0; i <= 5; i++){
        GPIOC->BRR = PINC13;
        delay_ms(stateMachine->reaction_time / 2);
        GPIOC->BSRR = PINC13;
        delay_ms(stateMachine->reaction_time/ 2);
      }

      delay_ms(500);
      stateMachine->current_state = IDLE;
      break;

    case TOO_EARLY:
      for (int i = 0; i < 3; i++){
        GPIOC->BSRR = PINC13;
        delay_ms(100);
        GPIOC->BRR = PINC13;
        delay_ms(100);
      }
      
      delay_ms(500);
      stateMachine->current_state = IDLE;
      break;

    default:
      stateMachine->current_state = IDLE;
      break;
  }
}
