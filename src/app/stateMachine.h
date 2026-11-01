#ifndef STATEMACHINE_H
#define STATEMACHINE_H

include <stdint.h>

// Event Structure 
typedef struct Event{
  uint8_t msg;
  uint32_t param;
} event_t;

typedef enum Event{
  EV_None = 0,
  EV_Pot_Change,
  EV_Button_Pressed,
  EV_Tick 
} event;

typedef enum State_Machine {
  STATE_TOGGLE_LED = 0,
  STATE_UPTIME,
  STATE_IMG
} state_machine;

typdef enum Machine {
  state_machine current_state;
  uint8_t led_status;
  uint8_t img_index;
  uint32_t uptime_seconds;
} Machine;

#endif
