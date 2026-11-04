#include "button.h"
#include "exti.h"
#include "clock.h"
#include "stm32f103.h"
#include "port_pin.h"
#include "watchdog.h"

#define BUTTON_DEBOUNCE_MS 300
#define BUTTON_HOLD_MS 1000

static uint32_t last_valid_time = 0;

button_event button_poll(void){
  supervisor_kick_bit(FLAG_BUTTON);

  // Triggered if PA1 is pressed 
  if (exti_flag){
    exti_flag = 0;

    if ((interrupt_timestamp - last_valid_time) >= BUTTON_DEBOUNCE_MS){
      last_valid_time = interrupt_timestamp;

      uint32_t hold_start = millis();
      uint8_t is_held = 0;

      while ((GPIOA->IDR & PINA1) == 0){
        if ((millis() - hold_start) >= BUTTON_HOLD_MS){
          is_held = 1;
          break;
        }
      }
      if (is_held){
        return BTN_EVENT_HOLD;
      } else{
        return BTN_EVENT_CLICK;
      }
    }
  }
  return BTN_EVENT_NONE;
}
