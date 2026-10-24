#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

// Returns 1 once per accepted press of the PA0 button, otherwise 0.
// Presses closer than BUTTON_DEBOUNCE_MS to the last accepted one are ignored.
uint8_t button_poll(void);

#endif
