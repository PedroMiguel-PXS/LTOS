// src/window_creator/WC_func.h

#ifndef WC_FUNC_H
#define WC_FUNC_H

#include <stdint.h>
#include <stddef.h>

#define WINDOW_BG_COLOR 7
#define WINDOW_TITLE_BG 8
#define WINDOW_TITLE_TEXT 15
#define WINDOW_BORDER_COL 0

void draw_window(int x, int y, int w, int h, const char* title);
void wc_put_text(const char* text, int x, int y, uint8_t color);
void wc_fill_rect(int x, int y, int w, int h, uint8_t color);

#endif