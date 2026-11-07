#include "uart_boot.h"
#include "../src/drivers/uart.h"
#include "../src/drivers/clock.h"
#include "flash.h"

#define META_START_ADDRESS 0x08001000
#define APP_START_ADDRESS  0x08001400 
#define BOOT_MAGIC         0x424F4F54 
#define VERSION_VAL        0x00010000

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

// Write 32 bit workd to flash 
static void flash_write_word(uint32_t address, uint32_t data){
  uint16_t low = (uint16_t)(data & 0xFFFF);
  uint16_t high = (uint16_t)(data >> 16);
  flash_write_halfword(address, low);
  flash_write_halfword(address + 2, high);
}

void uart_boot_protocol_loop(void){
  char cmd_buf[64];
  static uint32_t current_app_size = 0;
  static uint32_t current_app_crc = 0;
  static uint32_t target_flash_addr = APP_START_ADDRESS;

  while(1){
    int len = uart_boot_read_line(cmd_buf, sizeof(cmd_buf), 5000);
  
    if (len > 0){
      // HELLO HANDSHAKE
      if (cmd_buf[0] == 'H' && cmd_buf[1] == 'E' && cmd_buf[2] == 'L' && cmd_buf[3] == 'L' && cmd_buf[4] == 'O') {
        const char *reply = "BOOT_V1.0\n";
        for (int i = 0; reply[i] != '\0'; i++) uart_boot_putc(reply[i]);
      }
      // Wipes metadata page first so half-done updates are invalid
      else if (cmd_buf[0] == 'E' && cmd_buf[1] == 'R' && cmd_buf[2] == 'A' && cmd_buf[3] == 'S' && cmd_buf[4] == 'E') {
        if (flash_unlock() == FLASH_OK) {
          flash_erase_page(META_START_ADDRESS);
          
          for (uint32_t addr = APP_START_ADDRESS; addr < (APP_START_ADDRESS + 0x6000); addr += 1024) {
              flash_erase_page(addr);
          }
          
          flash_lock();
          target_flash_addr = APP_START_ADDRESS;
          current_app_size = 0;
          
          const char *reply = "ACK_ERASE\n";
          for (int i = 0; reply[i] != '\0'; i++) uart_boot_putc(reply[i]);
        } else {
          const char *reply = "ERR_UNLOCK\n";
          for (int i = 0; reply[i] != '\0'; i++) uart_boot_putc(reply[i]);
        }
      }
      // Receives binary chunk and writes to flash
      else if (cmd_buf[0] == 'D' && cmd_buf[1] == 'A' && cmd_buf[2] == 'T' && cmd_buf[3] == 'A') {
        // Expected format from Python: DATA <length> followed by raw bytes
      }
      // Writes metadata last: Magic, Size, CRC32, Version
      else if (cmd_buf[0] == 'F' && cmd_buf[1] == 'I' && cmd_buf[2] == 'N' && cmd_buf[3] == 'I' && cmd_buf[4] == 'S' && cmd_buf[5] == 'H') {
        // Parse size and CRC32 sent with finish command
        if (flash_unlock() == FLASH_OK) {
          // Write metadata matching generate_meta.py: Magic, Size, CRC32, Version
          flash_write_word(META_START_ADDRESS + 0, BOOT_MAGIC);
          flash_write_word(META_START_ADDRESS + 4, current_app_size);
          flash_write_word(META_START_ADDRESS + 8, current_app_crc);
          flash_write_word(META_START_ADDRESS + 12, VERSION_VAL);
          
          flash_lock();

          const char *reply = "ACK_FINISH\n";
          for (int i = 0; reply[i] != '\0'; i++) uart_boot_putc(reply[i]);
        }
      }
    }
  }
}
