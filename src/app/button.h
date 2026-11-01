#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

typedef enum{
  BTN_EVENT_NONE = 0,
  BTN_EVENT_CLICK,
  BTN_EVENT_HOLD 
} button_event;

// Returns 1 once per accepted press of the PA0 button, otherwise 0.
// Presses closer than BUTTON_DEBOUNCE_MS to the last accepted one are ignored.
button_event button_poll(void);

#endif
