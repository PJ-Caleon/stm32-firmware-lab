#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

#define SHELL_LINE_MAX 64
#define SHELL_MAX_ARGS 4
#define LED_ON_TIME_MS 3000

static char line_buf[SHELL_LINE_MAX];
static uint8_t line_idx = 0;
static char prev_char = 0;          // lets us treat "\r\n" as one Enter, not two

static uint8_t led_timer_active = 0;
static uint32_t led_off_time = 0;

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
  uart_puts("  bugs <n>    - Triggers 6 failures one at a time\r\n");
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
  printf("System Uptime: %lu seconds (%lu ms)\r\n", ms / 1000, ms);
}

static void cmd_pot(int argc, char **argv){
  (void)argc; (void)argv;
  printf("Potentiometer Vout: %u mV\r\n", Potentiometer_ReadRaw());
}

static void cmd_temp(int argc, char **argv){
  (void)argc; (void)argv;
  printf("MCU Temp: ~%ld °C | VREF Raw: %u \r\n", 
         temperature_ReadCelsius(), internalVref_ReadRaw());
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
  printf("PWM Loop, Avg Raw: %lu | Duty: %lu / 1000\r\n", average, measured_duty);
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

static void trigger_recursion(void){
  volatile uint32_t stack_filler[8];
  (void)stack_filler;
  trigger_recursion();
}

static void cmd_bugs(int argc, char **argv){
  if (argc < 2){
    uart_puts("Usage: bugs <1-5>\r\n");
    uart_puts("  1: Invalid Address Write (BusFault)\r\n");
    uart_puts("  2: Runaway Recursion (HardFault)\r\n");
    uart_puts("  3: Unaligned Multi-Word Access (UsageFault)\r\n");
    uart_puts("  4: Integer Divide by Zero (UsageFault)\r\n");
    uart_puts("  5: Bad Function Pointer 0xDEADBEEF (UsageFault)\r\n");
    uart_puts("  6: Trigger manual ASSERT \r\n");
    return;
  }
  char *bug_type = argv[1];

  if (bug_type[0] == '1' || strcmp(bug_type, "invalid") == 0){
    uart_puts("Triggering invalid address write (0x30000000)...\r\n");
    volatile uint32_t *invalid_ptr = (volatile uint32_t *)0x30000000;
    *invalid_ptr = 0xDEADBEEF;
  }
  else if (bug_type[0] == '2' || strcmp(bug_type, "recursion") == 0) {
    uart_puts("Triggering runaway recursion...\r\n");
    trigger_recursion();
  }
  else if (bug_type[0] == '3' || strcmp(bug_type, "unaligned") == 0) {
    uart_puts("Triggering unaligned multi-word access (0x20000001)...\r\n");
    volatile uint32_t *unaligned_ptr = (volatile uint32_t *)0x20000001;
    volatile uint32_t val = *unaligned_ptr;
    (void)val;
  }
  else if (bug_type[0] == '4' || strcmp(bug_type, "div0") == 0) {
    uart_puts("Triggering integer divide by zero (10 / 0)...\r\n");
    volatile int x = 10;
    volatile int y = 0;
    volatile int z = x / y;
    (void)z;
  }
  else if (bug_type[0] == '5' || strcmp(bug_type, "badptr") == 0) {
    uart_puts("Triggering call through bad function pointer (0xDEADBEEF)...\r\n");
    void (*bad_func)(void) = (void (*)(void))0xDEADBEEF;
    bad_func();
  }
  else if (strcmp(argv[1], "6") == 0){
    uart_puts("Triggering manual ASSER... \r\n");
    int val = 0;
    ASSERT(val == 1);
  }
  else {
    uart_puts("Unknown bug type. Use bugs 1 through 6.\r\n");
  }
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
  {"bugs", cmd_bugs}
};

#define NUM_COMMANDS (sizeof(cmd_table) / sizeof(cmd_table[0]))

// Split the line into words and run the matching command
static void shell_run_line(char *line){
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
