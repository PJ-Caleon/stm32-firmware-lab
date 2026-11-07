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

int uart_boot_read_line(char *buf, uint32_t max_len, uint32_t timeout_ms){
  uint32_t idx = 0;
  uint8_t c;
  uint32_t start = millis();

  while(idx < (max_len - 1)){
    uint32_t elapsed = millis() - start;
    if (timeout_ms != 0 && elapsed >= timeout_ms){
      break;
    }
    uint32_t remaining = (timeout_ms != 0) ? (timeout_ms - elapsed) : 0;

    if (uart_boot_getc(&c, remaining) == 0){
      if (c == '\n' || c == '\r'){
        if (idx > 0){
          break;
        }
        continue;
      }
      buf[idx++] = (char)c;
    } else{
      break;
    }
  }
  buf[idx] = '\0';
  return idx;
}

void uart_boot_protocol_loop(void){
  char cmd_buf[32];
  while(1){
    // wait for command from python update script 
    int len = uart_boot_read_line(cmd_buf, sizeof(cmd_buf), 2000);
  
    if (len > 0){
      // check for HELLO command 
      if (cmd_buf[0] == 'H' && cmd_buf[1] == 'E' && cmd_buf[2] == 'L' && cmd_buf[3] == 'L' && cmd_buf[4] == 'O') {
        const char *version_reply = "BOOT_V1.0\n";
        for (int i = 0; version_reply[i] != '\0'; i++){
          uart_boot_putc(version_reply[i]);
        }
      }
    }
  }
}
