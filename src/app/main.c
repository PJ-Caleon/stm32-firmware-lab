#include <stdint.h>

#include "clock.h"
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

int main(void){

  // Core timing
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

  // Application
  shell_init();
  oled_init();

  while(1){
    shell_poll();

    uint8_t btn_event = button_poll();

    char rx_data;
    if(ring_buffer_dequeue(&event_ring_buffer, &rx_data)){

    }

    oled_dashboard(ADC_GetScanPotRaw());
  }
}
