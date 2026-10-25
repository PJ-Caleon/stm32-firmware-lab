#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "uart.h"
#include "clock.h"
#include "port_pin.h"
#include "potentiometer.h"
#include "temperature.h"

/*
  Commands:
    status  - prints system uptime
    led on  - turns the LED on, it turns itself off after 3 seconds
    pot     - tells the current voltage mV the potentiometer output has 

  The shell only talks through uart.h (uart_read_char / uart_puts / printf)
  and port_pin.h (led_on / led_off). No registers here.
*/

#define SHELL_LINE_MAX 64
#define SHELL_MAX_ARGS 4
#define LED_ON_TIME_MS 3000

static char line_buf[SHELL_LINE_MAX];
static uint8_t line_idx = 0;
static char prev_char = 0;          // lets us treat "\r\n" as one Enter, not two

static uint8_t led_timer_active = 0;
static uint32_t led_off_time = 0;

// Commands 
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

typedef void (*cmd_func_t)(int argc, char **argv);

typedef struct {
  const char *name;
  cmd_func_t func;
} cmd_entry_t;

static const cmd_entry_t cmd_table[] = {
  {"status", cmd_status},
  {"led",    cmd_led},
  {"pot", cmd_pot},
  {"temp", cmd_temp}
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
  uart_puts("\r\nSTM32 Shell. Commands: status | led on | pot | temp \r\n");
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
