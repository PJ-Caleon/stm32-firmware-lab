#include <stdint.h>

#include "clock.h"
#include "watchdog.h"
#include "port_pin.h"
#include "exti.h"
#include "uart.h"
#include "button.h"
#include "potentiometer.h"
#include "stateMachine.h"
#include "shell.h"
#include "adc.h"
#include "temperature.h"
#include "pwm.h"
#include "i2c.h"
#include "oled.h"
#include "spi.h"
#include "fault_handler.h"
#include "ring_buffer.h"
#include "stm32f103.h"

int main(void){
  SCB->VTOR = 0x08001400;

  uint8_t is_iwdg_reset = (RCC->CSR & (1 << 29)) ? 1 : 0;
  CLEAR_BIT(RCC->CR, (1 << 24));

  // Core timing
  watchdog_init();
  setClock();
  systick_init();
  ring_buffer_init(&event_ring_buffer, event_buffer_storage, EVENT_BUF_SIZE);
  dwt_init();
  ADC_init();
  pwm_init();
  temperature_init();
  fault_handler_init();

  // Hardware
  port_pin_init();        // every pin + GPIO/AFIO clocks
  uart_init(PCLK2_HZ);    // USART1 + its DMA channels
  exti1_init();           // PA1 button interrupt
  exti1_latency_test();   // optional: fills dwt_start / dwt_isr for the debugger
  i2c_init();          
  spi_init();

  if (is_iwdg_reset){
    uart_puts("\r\nIWDG Reset Triggered\r\n");
  } else{
    uart_puts("\r\nNormal Power-On Reset\r\n");
  }

  // Watchdog config 
  uint32_t last_supervisor_run = 0;

  // Application
  shell_init();
  oled_init();

  // State Machine 
  Machine app_state;
  state_machine_init(&app_state);

  while(1){
    shell_poll();
    oled_poll();

    uint8_t btn_event = button_poll();
    uint16_t pol_val = ADC_GetScanPotRaw();

    state_machine_run(&app_state, pol_val, btn_event);
   
    // run supervisor every 1000ms (1 Hz)
    if ((millis() - last_supervisor_run) >= 1000){
      last_supervisor_run = millis();
      supervisor_task();
    }

  }
}
