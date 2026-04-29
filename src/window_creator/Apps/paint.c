// here will be paint.c
// we will paint with the mouse
// use the mouse position to draw on the screen
// use the mouse buttons to draw
// use button "X" to close window
// code

// we will use WC_func.h
// lets use diff path for include
// path is src/window_creator/WC_func.h
#include "../../window_creator/WC_func.h"
#include <stdint.h>

extern void putpixel(int x, int y, uint32_t color);
extern void terminal_putentryat(char c, uint8_t color, int x, int y);
extern int mouse_x, mouse_y;
extern int mouse_byte[3];
uint32_t color = 0x00AA0000;

void paint_draw_brush(int x0, int y0, int radius, uint32_t color) {
  for (int y = -radius; y <= radius; y++) {
    for (int x = -radius; x <= radius; x++) {
      if (x * x + y * y <= radius * radius) {
        putpixel(x0 + x, y0 + y, color);
      }
    }
  }
}

void draw_paint(int x, int y) {
  draw_window(x, y, 30, 20, "Paint");
  // if left mouse button is pressed
  if (mouse_byte[0] & 0x01) {
    int win_px = x * 8;
    int win_py = y * 8;
    if (mouse_x > win_px + 8 && mouse_x < win_px + 232 &&
        mouse_y > win_py + 16 && mouse_y < win_py + 152) {
      paint_draw_brush(mouse_x, mouse_y, 2, color);
      terminal_putentryat(' ', 0 | 4 << 4, x + 2, y + 19);  // Red (4)
      terminal_putentryat(' ', 0 | 1 << 4, x + 4, y + 19);  // Blue (1)
      terminal_putentryat(' ', 0 | 2 << 4, x + 6, y + 19);  // Green (2)
      terminal_putentryat(' ', 0 | 14 << 4, x + 8, y + 19); // Yellow (14)
      // mouse logic
      if (mouse_byte[0] & 0x01) {
        int mx = mouse_x / 8;
        int my = mouse_y / 8;
        if (my == y + 19) {
          if (mx == x + 2) {
            color = 0x00AA0000;
          }
          if (mx == x + 4) {
            color = 0x000000AA;
          }
          if (mx == x + 6) {
            color = 0x0000AA00;
          }
          if (mx == x + 8) {
            color = 0x00FFFF55;
          }
        }
      }
    }
  }
}
