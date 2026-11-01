#include <string.h>
#include "oled_frame.h"
#include "oled.h"

static void draw_horizontal_line(uint8_t x0, uint8_t x1, uint8_t y){
  for(uint8_t x = x0; x <= x1; x++){
    oled_draw_pixel(x,y,1);
  }
}

static void draw_vertical_line(uint8_t x, uint8_t y0, uint8_t y1){
  for(uint8_t y = y0; y <= y1; y++){
    oled_draw_pixel(x,y,1);
  }
}

void oled_draw_frame(const char *title){
  // Compute title width, 6 pixels per char using 5x7 font 
  uint8_t title_len = strlen(title);
  uint8_t title_px_width = title_len * 6;

  // Frame boundaries 
  uint8_t x_min = 4;
  uint8_t x_max = 123;
  uint8_t y_min = 4;
  uint8_t y_max = 59;

  uint8_t title_x = (128 - title_px_width) / 2;

  if (title_x > x_min + 1){
    draw_horizontal_line(x_min, title_x - 2, y_min);
    draw_horizontal_line(title_x + title_px_width + 2, x_max, y_min);
  } else{
    draw_horizontal_line(x_min, x_max, y_min);
  }
  // Draw title text at top
  oled_draw_string(title_x, y_min - 3, title);

  draw_horizontal_line(x_min, x_max, y_max);
  draw_vertical_line(x_min, y_min,y_max);
  draw_vertical_line(x_max, y_min,y_max);
}

void oled_draw_centered_text(uint8_t y, const char *text){
  uint8_t len = strlen(text);
  uint8_t px_width = len * 6;
  uint8_t  x = (128 - px_width) / 2;
  oled_draw_string(x,y,text);
}
