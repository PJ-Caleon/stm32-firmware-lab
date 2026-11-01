#ifndef OLED_FRAME_H
#define OLED_FRAME_H

#include <stdint.h>

void oled_draw_frame(const char *title);
void oled_draw_centered_text(uint8_t y, const char *text);

#endif
