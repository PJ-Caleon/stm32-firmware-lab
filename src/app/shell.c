#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "startup.h"
#include "shell.h"
#include "uart.h"
#include "clock.h"
#include "port_pin.h"
#include "potentiometer.h"
#include "temperature.h"
#include "pwm.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "stm32f103.h"
#include "fault_handler.h"
#include "watchdog.h"
#include "eeprom_config.h"
#include "flash.h"
#include "crc.h"

#define SHELL_LINE_MAX 64
#define SHELL_MAX_ARGS 4
#define LED_ON_TIME_MS 3000

static char line_buf[SHELL_LINE_MAX];
static uint8_t line_idx = 0;
static char prev_char = 0;          // lets us treat "\r\n" as one Enter, not two

static uint8_t led_timer_active = 0;
static uint32_t led_off_time = 0;

static AppConfig cached_config;

static bool config_load(AppConfig *config_out){
  if (EEPROM_Read(config_out)){
    cached_config = *config_out;
    return true;
  }
  
  // default
  config_out->device_id = 0x0001;
  config_out->baud_rate = 115200;
  strcpy(config_out->device_name, "STM32");
  config_out->calibration_mode = 0;

  cached_config = *config_out;
  return false;
}

static bool config_save(const AppConfig *config_in){
  if (EEPROM_Write(config_in)){
    cached_config = *config_in;
    return true;
  }
  return false;
}

// Commands 

static void cmd_help(int argc, char **argv){
  (void)argc; (void)argv;
  uart_puts("Available commands:\r\n");
  uart_puts("  status      - Print system uptime\r\n");
  uart_puts("  led on      - Turn on onboard LED for 3 seconds\r\n");
  uart_puts("  pot         - Read potentiometer voltage in mV\r\n");
  uart_puts("  temp        - Read MCU internal temperature and VREF\r\n");
  uart_puts("  pwm         - Update and print hardware PWM duty cycle\r\n");
  uart_puts("  pwm_test    - Run 1000-sample loopback test (PA6 to PA2)\r\n");
  uart_puts("  pwm_led_dim - Run 20-second interactive LED dimming via pot\r\n");
  uart_puts("  i2c_scan    - Scans the I2C bus for any connection and output addr\r\n");
  uart_puts("  spi_test    - Send bytes 0 to 255 and expect them back unchanged\r\n");
  uart_puts("  hang        - cause a hang to trigger watchdog\r\n");
  uart_puts("  ram         - Prints the .data, .bss, free RAM and peak stack use in bytes\r\n");
  uart_puts("  config      - Adjust the EEPROM memory\r\n");
  uart_puts("  config_fail - Simulate a write (name = fail) and cutoff midway, never reaching CRC\r\n");
}

static void cmd_led(int argc, char **argv){
  if (argc == 2 && strcmp(argv[1], "on") == 0){
    led_on();
    led_off_time = millis() + LED_ON_TIME_MS;
    led_timer_active = 1;
    uart_puts("LED on for 3 seconds\r\n");
  } else{
    uart_puts("Usage: led on\r\n");
  }
}

static void cmd_status(int argc, char **argv){
  (void)argc; (void)argv;
  uint32_t ms = millis();
  printf("System Uptime: %lu seconds (%lu ms)\r\n", (unsigned long)(ms / 1000), (unsigned long)(ms));
}

static void cmd_pot(int argc, char **argv){
  (void)argc; (void)argv;
  printf("Potentiometer Vout: %u mV\r\n", Potentiometer_ReadRaw());
}

static void cmd_temp(int argc, char **argv){
  (void)argc; (void)argv;
  printf("MCU Temp: ~%ld °C | VREF Raw: %u \r\n", 
         (long)temperature_ReadCelsius(), internalVref_ReadRaw());
}

static void cmd_pwm(int argc, char **argv){
  (void)argc; (void)argv;
  printf("PWM Duty Cycle Set: %u / 1000\r\n", pwm_update());
}

static void cmd_pwm_test(int argc, char **argv){
  (void)argc; (void)argv;
  uint32_t sum = 0;
  int samples = 1000;

  for (int i = 0; i < samples; i++){
    sum += ADC_GetScanPotRaw();
  }

  uint32_t average = sum/ samples;
  uint32_t measured_duty = (average * 1000UL)/ 4095UL;
  printf("PWM Loop, Avg Raw: %lu | Duty: %lu / 1000\r\n", (unsigned long)average, (unsigned long)measured_duty);
}

static void cmd_led_dim(int argc, char **argv){
  (void)argc; (void)argv;

  printf("Starting 20-second LED dimming test. Adjust the potentiometer...\r\n");

  for (int cycle = 0; cycle < 2000; cycle++){
    uint16_t raw = ADC_GetScanPotRaw();
    uint32_t duty = ((uint32_t)raw * 1000UL)/ 4095UL;

    uint32_t on_time_ms = (1000UL - duty) * 10UL/ 1000UL;
    uint32_t off_time_ms = 10UL - on_time_ms;

    if (on_time_ms > 0){
      led_on();
      delay_ms(on_time_ms);
    }
    if (off_time_ms > 0){
      led_off();
      delay_ms(off_time_ms);
    }
    shell_poll();
    watchdog_pat();
  }
  led_off();
  printf("LED dimming test completed\r\n");
}

static void cmd_i2c_scan(int argc, char **argv){
  (void)argc; (void)argv;
  i2c_scan();
}

static void cmd_spi_test(int argc, char **argv){
  (void)argc; (void)argv;
  
  uint8_t tx_buffer[256];
  uint8_t rx_buffer[256];
  for (int i = 0; i <= 255; i++){
    tx_buffer[i] = (uint8_t)i;
  }

  uint8_t br_dividers[] = {5, 3, 1};
  const char *br_names[] = {"div_64", "div_16", "div_4"};

  uart_puts("Starting SPI Loopback test (bytes 0 to 255)...\r\n");

  for (int s = 0; s < 3; s++){
    CLEAR_BIT(SPI1->CR1, (1 << 6));
    CLEAR_BIT(SPI1->CR1, (7 << 3));
    SET_BIT(SPI1->CR1, (br_dividers[s] << 3));
    SET_BIT(SPI1->CR1, (1 << 6));

    printf("Running spi_xfer at %s... ", br_names[s]);

    // Clear receive buffer
    for (int i = 0; i < 256; i++) {
      rx_buffer[i] = 0;
    }

    spi_xfer(tx_buffer, rx_buffer, 256);

    // Verify results
    int errors = 0;
    for (int i = 0; i < 256; i++) {
      if (rx_buffer[i] != tx_buffer[i]) {
        errors++;
      }
    }

    if (errors == 0) {
      uart_puts("PASSED\r\n");
    } else {
      printf("FAILED (%d mismatches)\r\n", errors);
    }
  }
  uart_puts("SPI Loopback Test Complete\r\n");
}

static void cmd_hang(int argc, char **argv){
  (void)argc; (void)argv;

  uart_puts("Hang is starting...\r\n");
  delay_ms(5000);
}

static void cmd_ram(int argc, char **argv){
  (void)argc; (void)argv;
  // prints .data, .bss, free RAM, peak stack in bytes 
  uint32_t data_size = (uint32_t)((uintptr_t) &_edata) - (uint32_t)((uintptr_t) &_sdata);
  printf(".data: %lu bytes\r\n", (unsigned long)data_size);
 
  uint32_t bss_size = (uint32_t)((uintptr_t) &_ebss) - (uint32_t)((uintptr_t) &_sbss);
  printf(".bss: %lu bytes\r\n", (unsigned long)bss_size);

  uint32_t free_ram_size = (uint32_t)((uintptr_t) &_estack) - (uint32_t)((uintptr_t) &_ebss);
  printf("Free Ram: %lu bytes\r\n", (unsigned long)free_ram_size);

  uint32_t *ptr = &__stack_limit;
  uint32_t unused_bytes = 0;

  while(ptr < &_estack && *ptr == 0xDEADBEEF){
    unused_bytes += 4;
    ptr++;
  }

  uint32_t total_stack = (uint32_t)((uintptr_t)&_estack - (uintptr_t)&__stack_limit);
  uint32_t peak_stack = total_stack - unused_bytes;

  printf("Peak Stack: %lu out of %lu bytes\r\n", (unsigned long)peak_stack, (unsigned long)total_stack);
}

static void cmd_config(int argc, char **argv){
  AppConfig cfg;
  config_load(&cfg);

  if (argc < 2){
    uart_puts("config show - Display current device config\r\n");
    uart_puts("config set name <str> - Display current device config\r\n");
    return;
  }
  
  if (strcmp(argv[1], "show") == 0) {
    printf("Device Name : %s\r\n", cfg.device_name);
    printf("Device ID   : 0x%04lX\r\n", (unsigned long)cfg.device_id);
    printf("Baud Rate   : %lu\r\n", (unsigned long)cfg.baud_rate);
  }
  else if (strcmp(argv[1], "set") == 0 && argc >= 4) {
    if (strcmp(argv[2], "name") == 0) {
      strncpy(cfg.device_name, argv[3], sizeof(cfg.device_name) - 1);
      cfg.device_name[sizeof(cfg.device_name) - 1] = '\0';
      
      if (config_save(&cfg)) {
        printf("Saved Device Name -> %s\r\n", cfg.device_name);
      } else {
        uart_puts("Error: Failed to write configuration to flash.\r\n");
      }
    } else {
      uart_puts("Unknown property. Use 'config set name <str>'.\r\n");
    }
  } else {
    uart_puts("Invalid arguments. Type 'config' for usage.\r\n");
  }
}

static void cmd_config_fail(int argc, char **argv) {
    (void)argc; (void)argv;
    
    AppConfig cfg;
    config_load(&cfg);

    // Modify the name to a test string to show what would have been written
    strncpy(cfg.device_name, "fail", sizeof(cfg.device_name) - 1);
    cfg.device_name[sizeof(cfg.device_name) - 1] = '\0';

    uint32_t next_seq = 1;
    const uint32_t pages[NUM_PAGES] = {CONFIG_PAGE_ADDR1, CONFIG_PAGE_ADDR2};
    uint32_t target_slot_addr = 0;
    bool slot_found = false;

    for (int p = 0; p < NUM_PAGES; p++) {
        uint32_t addr = pages[p];
        uint32_t page_end = addr + CONFIG_SIZE;

        while (addr + sizeof(ConfigRecord) <= page_end) {
            ConfigRecord *candidate = (ConfigRecord *)(uintptr_t)addr;
            
            if (candidate->magic == 0xFFFFFFFF) {
                target_slot_addr = addr;
                slot_found = true;
                break;
            }

            if (candidate->magic == CONFIG_MAGIC) {
                size_t crc_data_len = offsetof(ConfigRecord, crc32);
                if (crc32_bit_by_bit((const uint8_t *)candidate, crc_data_len, 0) == candidate->crc32) {
                    if (candidate->sequence_number >= next_seq) {
                        next_seq = candidate->sequence_number + 1;
                    }
                }
            }
            addr += sizeof(ConfigRecord);
        }
        if (slot_found) break;
    }

    if (!slot_found) {
        target_slot_addr = CONFIG_PAGE_ADDR1;
        if (flash_unlock() == FLASH_OK) {
            flash_erase_page(CONFIG_PAGE_ADDR1);
            flash_erase_page(CONFIG_PAGE_ADDR2);
            flash_lock();
        }
    }

    ConfigRecord new_record;
    new_record.magic = CONFIG_MAGIC;
    new_record.sequence_number = next_seq;
    new_record.config = cfg;
    
    // Compute valid CRC but we WON'T flash it
    size_t crc_data_len = offsetof(ConfigRecord, crc32);
    new_record.crc32 = crc32_bit_by_bit((const uint8_t *)&new_record, crc_data_len, 0);

    // Write ONLY the body bytes (Stop before writing the CRC field)
    if (flash_unlock() != FLASH_OK) {
        uart_puts("Error: Flash unlock failed.\r\n");
        return;
    }

    const uint16_t *data_ptr = (const uint16_t *)&new_record;
    size_t crc_field_offset = offsetof(ConfigRecord, crc32);

    for (uint32_t i = 0; i < crc_field_offset; i += 2) {
        watchdog_pat();
        uint16_t half_word = *data_ptr++;
        if (flash_write_halfword(target_slot_addr + i, half_word) != FLASH_OK) {
            flash_lock();
            uart_puts("Error: Flash write failed during body write.\r\n");
            return;
        }
    }

    flash_lock();

    uart_puts("[TEST] Body written successfully. Skipping CRC write...\r\n");
    uart_puts("[TEST] Forcing immediate system reset now!\r\n");
}

typedef void (*cmd_func_t)(int argc, char **argv);

typedef struct {
  const char *name;
  cmd_func_t func;
} cmd_entry_t;

static const cmd_entry_t cmd_table[] = {
  {"help", cmd_help},
  {"status", cmd_status},
  {"led",    cmd_led},
  {"pot", cmd_pot},
  {"temp", cmd_temp},
  {"pwm", cmd_pwm},
  {"pwm_test", cmd_pwm_test},
  {"pwm_led_dim", cmd_led_dim},
  {"i2c_scan", cmd_i2c_scan},
  {"spi_test", cmd_spi_test},
  {"hang", cmd_hang},
  {"ram", cmd_ram},
  {"config", cmd_config},
  {"config_fail", cmd_config_fail}
};

#define NUM_COMMANDS (sizeof(cmd_table) / sizeof(cmd_table[0]))

// Split the line into words and run the matching command
void shell_run_line(char *line){
  char *argv[SHELL_MAX_ARGS];
  int argc = 0;

  char *token = strtok(line, " ");
  while (token != NULL && argc < SHELL_MAX_ARGS){
    argv[argc++] = token;
    token = strtok(NULL, " ");
  }

  if (argc == 0){
    return;
  }

  for (size_t i = 0; i < NUM_COMMANDS; i++){
    if (strcmp(argv[0], cmd_table[i].name) == 0){
      cmd_table[i].func(argc, argv);
      return;
    }
  }
  printf("Unknown command: %s. Try: status | led on\r\n", argv[0]);
}

// Turns the LED off once its 3 seconds are up (no blocking delay)
static void shell_led_timer(void){
  if (led_timer_active && (int32_t)(millis() - led_off_time) >= 0){
    led_off();
    led_timer_active = 0;
  }
}

void shell_init(void){
  line_idx = 0;
  uart_puts("\r\nSTM32 Shell. Type 'help' for list of commands \r\n");
  uart_puts("✿ ~ ❯ ");
}

void shell_poll(void){
  shell_led_timer();

  supervisor_kick_bit(FLAG_SHELL);

  while (uart_data_available()){
    char c = uart_read_char();

    // Enter key (Carriage Return or Newline)
    if (c == '\r' || c == '\n'){
      int second_half_of_crlf = (c == '\n' && prev_char == '\r');
      prev_char = c;
      if (second_half_of_crlf){
        continue;
      }

      uart_puts("\r\n");
      line_buf[line_idx] = '\0';
      if (line_idx > 0){
        shell_run_line(line_buf);
      }
      line_idx = 0;
      uart_puts("✿ ~ ❯  ");
      continue;
    }

    prev_char = c;

    // Backspace
    if (c == '\b' || c == 127){
      if (line_idx > 0){
        line_idx--;
        uart_puts("\b \b"); // Erase character on terminal screen
      }
    }
    // Printable characters: store and echo
    else if (line_idx < SHELL_LINE_MAX - 1 && c >= 32 && c <= 126){
      line_buf[line_idx++] = c;
      uart_putc(c);
    }
  }
}
