#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <stdint.h>
#include "ring_buffer.h"

#define EVENT_BUF_SIZE 32 

static char event_buffer_storage[EVENT_BUF_SIZE];
ring_buffer_t event_ring_buffer;


// Event Structure 
typedef struct{
  uint8_t msg;
  uint32_t param;
} event_t;

typedef enum{
  EV_None = 0,
  EV_Pot_Change,
  EV_Button_Pressed,
  EV_Tick 
} event;

typedef enum {
  STATE_TOGGLE_LED = 0,
  STATE_UPTIME,
  STATE_IMG
} state_machine_t;

typedef struct{
  state_machine_t current_state;
  uint8_t led_status;
  uint8_t img_index;
  uint32_t uptime_seconds;
} Machine;

#endif
