#include <stdio.h>
#include "stateMachine.h"
#include "clock.h"
#include "port_pin.h"
#include "oled.h"
#include "malfoid.h"
#include "button.h"
#include "oled_frame.h"
#include "haimiya.h"
#include "nixos.h"

char event_buffer_storage[EVENT_BUF_SIZE];
ring_buffer_t event_ring_buffer;

void state_machine_init(Machine *sm){
  sm->current_state = STATE_UPTIME;
  sm->led_status = 0;
  sm->img_index = 0;
  sm->uptime_seconds = 0;
  sm->is_blinking = 0;
  sm->last_blink_time = 0;
}

void state_machine_run(Machine *sm, uint16_t pot_val, uint8_t btn_pressed){
  char buf[32];

  // Store previous state to detect window switch 
  state_machine_t previous_state = sm->current_state;

  // Potentiometer Window 
  if (pot_val < 1365){
    sm->current_state = STATE_TOGGLE_LED;
  } else if (pot_val < 2730){
    sm->current_state = STATE_UPTIME;
  } else{
    sm->current_state = STATE_IMG;
  }

  // Detect if new window 
  uint8_t window_changed = (sm->current_state != previous_state);

  // Background Persist blinking
  uint8_t blink_ticked = 0;
  if (sm->is_blinking){
    if ((millis() - sm->last_blink_time) >= 500){
      sm->last_blink_time = millis();
      sm->led_status = !sm->led_status;
      if (sm->led_status){
        led_on();
      } else{
        led_off();
      }
      blink_ticked = 1;
    }
  }

  // Handle State Action
  switch (sm->current_state){
    case STATE_TOGGLE_LED:
      if (btn_pressed == BTN_EVENT_HOLD){
        sm->is_blinking = !sm->is_blinking;
        if(!sm->is_blinking){
          sm->led_status = 0;
          led_off();
        }
      } else if (btn_pressed == BTN_EVENT_CLICK){
        if (sm->is_blinking){
          sm->is_blinking = 0;
          sm->led_status = 0;
          led_off();
        } else{
          sm->led_status = !sm->led_status;
          if(sm->led_status){
            led_on();
          } else{
            led_off();
          }
        }
      }

      // LED UI
      if (window_changed || (btn_pressed != BTN_EVENT_NONE) || (sm->is_blinking && blink_ticked)){
        oled_clear();
        oled_draw_frame("LED");
        if (sm->is_blinking){
          sprintf(buf, "LED: BLINKING");
        } else{
        sprintf(buf, "LED:%s", sm->led_status ? "ON" : "OFF");
        }
        oled_draw_centered_text(28,buf);
        oled_refresh();
      }
      break;

    case STATE_UPTIME:
      sm->uptime_seconds = millis()/ 1000;

      // Uptime UI 
      oled_clear();
      oled_draw_frame("UPTIME");
      sprintf(buf, "RUNNING: %lu S", (unsigned long)sm->uptime_seconds);
      oled_draw_centered_text(28,buf);
      oled_refresh();
      break;

    case STATE_IMG:
      static uint8_t last_img_index = 0xFF; // Force initial draw 
      uint8_t image_changed = (sm->img_index != last_img_index);

      if (btn_pressed == BTN_EVENT_CLICK){
        sm->img_index = (sm->img_index+1) % 4;
      }

      // Image UI 
      if (window_changed || image_changed || (btn_pressed == BTN_EVENT_CLICK)){
        oled_clear();

        if (sm->img_index == 0){
          oled_draw_frame("IMAGE");
          oled_draw_centered_text(28, "PRESS BTN");
        } else if (sm->img_index == 1){
          draco_malfoid();
        } else if (sm->img_index == 2){
          haimiya_mio();
        } else if (sm->img_index == 3){
          nixos();
        }

        oled_refresh();
        last_img_index = sm->img_index;
      }
      break;

    default:
      break;
  }
}

