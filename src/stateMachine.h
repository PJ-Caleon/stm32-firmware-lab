#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <stdint.h>

typedef enum {
  IDLE,
  WAIT_RANDOM,
  GO,
  TOO_EARLY,
  RESULT
} State;

typedef struct {
  State current_state;
  uint32_t random_wait_target;
  uint32_t timer_start_tick;
  uint32_t reaction_time;
} StateMachine; 

void stateMachine_update(StateMachine *stateMachine, uint8_t button_pressed, uint32_t current_time);

#endif
