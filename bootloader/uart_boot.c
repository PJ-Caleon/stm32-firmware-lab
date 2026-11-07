#include "uart_boot.h"
#include "../src/drivers/uart.h"
#include "../src/drivers/clock.h"

extern uint32_t millis(void);

void uart_boot_init(uint32_t pclk_freq){
  uart_init(pclk_freq);
}

void uart_boot_putc(uint8_t byte){
  uart_putc((char)byte);
}

int uart_boot_getc(uint8_t *byte, uint32_t timeout_ms){
  uint32_t start = millis();

  // Poll DMA 
  while(!uart_data_available()){
    if(timeout_ms != 0 && (millis() - start) >= timeout_ms){
      return -1;
    }
  }

  *byte = (uint8_t)uart_read_char();
  return 0;
}

// Echo loop test 
void uart_boot_echo_loop(void){
  uint8_t rx_byte;
  while(1){
    if(uart_boot_getc(&rx_byte, 1000) == 0){
      uart_boot_putc(rx_byte);
    }
  }
}
