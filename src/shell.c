#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "stm32f103.h"
#include "clock.h"
#include "uart.h"

#define MAX_LINE_LENGTH 64
#define MAX_ARGS 4

/*
  This script is 100% vibe coded with corrections to variables, made just to have commands for UART to read and output 

  On boot after reset:
  (Out) STM32 Shell Initialized. Type 'help' for commands.
  shell> 

  led on  - turns on PC13 
  led off - turns off PC13 
  led - "Usage: led on | led off"
  Status - "System Uptime: X seconds (x ms)"
*/

static char line_buf[MAX_LINE_LENGTH];
static uint8_t line_idx = 0;

// Forward declarations of command functions
static void cmd_help(int argc, char **argv);
static void cmd_led(int argc, char **argv);
static void cmd_status(int argc, char **argv);

// Command table structure
typedef void (*cmd_func_t)(int argc, char **argv);

typedef struct {
    const char *name;
    cmd_func_t func;
    const char *help_text;
} cmd_entry_t;

// Static command registry table
static const cmd_entry_t cmd_table[] = {
    {"help",   cmd_help,   "Lists available commands"},
    {"led",    cmd_led,    "Control onboard LED: led on | led off"},
    {"status", cmd_status, "Prints system uptime"}
};

#define NUM_COMMANDS (sizeof(cmd_table) / sizeof(cmd_entry_t))

void shell_init(void) {
    line_idx = 0;
    memset(line_buf, 0, sizeof(line_buf));
    uart_puts("\r\nSTM32 Shell Initialized. Type 'help' for commands.\r\n");
    uart_puts("shell> ");
}

/*
  Non-blocking poller to be called inside your main while(1) loop.
  Reads from the RX ring buffer character by character.
*/
void shell_poll(void) {
    while (uart_data_available()) {
        char c = uart_read_char();

        // Handle Enter key (Carriage Return or Newline)
        if (c == '\r' || c == '\n') {
            uart_puts("\r\n");
            line_buf[line_idx] = '\0';

            if (line_idx > 0) {
                // Tokenize and parse line
                char *argv[MAX_ARGS];
                int argc = 0;
                char *token = strtok(line_buf, " ");
                
                while (token != NULL && argc < MAX_ARGS) {
                    argv[argc++] = token;
                    token = strtok(NULL, " ");
                }

                if (argc > 0) {
                    int found = 0;
                    for (int i = 0; i < NUM_COMMANDS; i++) {
                        if (strcmp(argv[0], cmd_table[i].name) == 0) {
                            cmd_table[i].func(argc, argv);
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        printf("Unknown command: %s. Type 'help'.\r\n", argv[0]);
                    }
                }
            }

            line_idx = 0;
            memset(line_buf, 0, sizeof(line_buf));
            uart_puts("shell> ");
        } 
        // Handle Backspace
        else if (c == '\b' || c == 127) {
            if (line_idx > 0) {
                line_idx--;
                uart_puts("\b \b"); // Erase character on terminal screen
            }
        } 
        // Handle standard printable characters
        else if (line_idx < MAX_LINE_LENGTH - 1 && c >= 32 && c <= 126) {
            line_buf[line_idx++] = c;
            uart_putc(c); // Echo character back to screen
        }
    }
}

// --- Command Handlers ---

static void cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    uart_puts("Available Commands:\r\n");
    for (int i = 0; i < NUM_COMMANDS; i++) {
        printf("  %-8s - %s\r\n", cmd_table[i].name, cmd_table[i].help_text);
    }
}

static void cmd_led(int argc, char **argv) {
    if (argc < 2) {
        uart_puts("Usage: led on | led off\r\n");
        return;
    }

    // Note: On standard Blue Pill boards, PC13 LED is active-LOW 
    // (Writing 0 turns it ON, writing 1 turns it OFF)
    if (strcmp(argv[1], "on") == 0) {
        CLEAR_BIT(GPIOC->ODR, (1 << 13)); // Turn LED ON
        uart_puts("LED turned ON\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        SET_BIT(GPIOC->ODR, (1 << 13));   // Turn LED OFF
        uart_puts("LED turned OFF\r\n");
    } else {
        printf("Invalid argument: %s. Use 'on' or 'off'.\r\n", argv[1]);
    }
}

static void cmd_status(int argc, char **argv) {
  (void)argc; (void)argv;
  uint32_t ms = millis();
  printf("System Uptime: %lu seconds (%lu ms)\r\n", ms / 1000, ms);
}
