#include "unity.h"
#include "shell.h"
#include "uart.h"
#include <string.h>
#include <stdio.h> 

static int led_state = 0;

void led_on(void) { led_state = 1; }
void led_off(void) { led_state = 0; }
uint32_t millis(void) { return 1000; }

void setUp(void){
  led_state = 0;
}
void tearDown(void){
  led_state = 0;
}

void test_readable_input(void){
  char input[] = "status";

  // Get the stdout to verify shell_run_line parses status
  (void)freopen("test_output.txt", "w+", stdout);
  shell_run_line(input);
  fflush(stdout);

  freopen("/dev/tty", "w", stdout);

  char buf[128] = {0};
  FILE *read_f = fopen("test_output.txt", "r");
  if (read_f) {
    fgets(buf, sizeof(buf), read_f);
    fclose(read_f);
  }
  TEST_ASSERT_NOT_NULL(strstr(buf, "System Uptime"));
}

void test_validate_cmd(void){
  char invalid_cmd[] = "power";

  (void)freopen("test_output.txt", "w+", stdout);
  shell_run_line(invalid_cmd);
  fflush(stdout);

  freopen("/dev/tty", "w", stdout);

  char buf[128] = {0};
  FILE *read_f = fopen("test_output.txt", "r");
  if (read_f) {
    fgets(buf, sizeof(buf), read_f);
    fclose(read_f);
  }

  TEST_ASSERT_NOT_NULL(strstr(buf, "Unknown command: power. Try: status | led on"));
}

void test_correctfunction_performed(void){
  led_state = 0;
  char cmd[] = "led on";
  shell_run_line(cmd);
  TEST_ASSERT_EQUAL_INT(1, led_state);
}


int main(void){
  UNITY_BEGIN();
  RUN_TEST(test_readable_input);
  RUN_TEST(test_validate_cmd);
  RUN_TEST(test_correctfunction_performed);
  return UNITY_END();
}
/*
  Check the data parser, raw inputted data is converted to structured 
  data the script can operate with
*/

void uart_puts(const char *s) { (void)s; }
void uart_putc(char c) { (void)c; }
int uart_data_available(void) { return 0; }
char uart_read_char(void) { return '\0'; }

uint16_t Potentiometer_ReadRaw(void) { return 2048; }
uint16_t internalVref_ReadRaw(void) { return 1200; }
int32_t temperature_ReadCelsius(void) { return 25; }
uint16_t pwm_update(void) { return 500; }
uint16_t ADC_GetScanPotRaw(void) { return 2048; }
void delay_ms(uint32_t ms) { (void)ms; }
void i2c_scan(void) {}
void spi_xfer(uint8_t *tx, uint8_t *rx, uint16_t len) { 
  if (tx && rx) {
    memcpy(rx, tx, len);
  }
}
void assert_failed(const uint8_t *file, uint32_t line) {
  (void)file;
  (void)line;
}
