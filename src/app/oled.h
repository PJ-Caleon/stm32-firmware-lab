#ifndef OLED_H
#define OLED_H 

#include <stdint.h>

void oled_init(void);
void oled_clear(void);
void oled_draw_pixel(uint8_t x, uint8_t y, uint8_t color);
void oled_draw_char(uint8_t x, uint8_t y, char c);
void oled_draw_string(uint8_t x, uint8_t y, const char *str);
void oled_refresh(void);
void oled_dashboard(uint32_t adc_val);

#endif
