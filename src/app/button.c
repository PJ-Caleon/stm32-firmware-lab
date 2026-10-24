#include "button.h"
#include "exti.h"

#define BUTTON_DEBOUNCE_MS 300

static uint32_t last_valid_time = 0;

uint8_t button_poll(void){
  // Triggered if PA0 is pressed 
  if (exti_flag){
    exti_flag = 0;

    if ((interrupt_timestamp - last_valid_time) >= BUTTON_DEBOUNCE_MS){
      last_valid_time = interrupt_timestamp;
      return 1;
    }
  }
  return 0;
}
