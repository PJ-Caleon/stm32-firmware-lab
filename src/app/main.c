#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
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

static void vAppTask(void *pvParameters);

int main(void){

  // Core timing
  setClock();
  fault_handler_init();
  ring_buffer_init(&event_ring_buffer, event_buffer_storage, EVENT_BUF_SIZE);
  dwt_init();
  ADC_init();
  pwm_init();
  temperature_init();

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

  // Create FreeRTOS application task with a safer stack size (512 words = 2KB)
  xTaskCreate(vAppTask, "AppTask", 128, NULL, tskIDLE_PRIORITY + 1, NULL);

  // Start the real-time scheduler
  vTaskStartScheduler();
}

static void vAppTask(void *pvParameters){
  (void)pvParameters;

  Machine app_state;
  state_machine_init(&app_state);
  uint8_t state_start = 0;

  const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10ms poll rate 
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1){
    shell_poll();

    uint8_t btn_event = button_poll();
    uint16_t pol_val = ADC_GetScanPotRaw();
  
    if (btn_event == BTN_EVENT_HOLD){
      state_start = 1;
    }

    if (state_start == 1){
      state_machine_run(&app_state, pol_val, btn_event);
    }

    // Yield control cooperatively using FreeRTOS ticks
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}
