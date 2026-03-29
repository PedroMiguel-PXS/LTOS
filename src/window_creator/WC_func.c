#include <stdint.h>
#include <stdbool.h>
#include "WC_func.h"

// import the video function of kernel
extern void terminal_putentryat(char c, uint8_t color, int x, int y);

// Generic function to draw a window with title
void draw_window(int x, int y, int w, int h, const char* title) {
    // Title Bar (Black Background with White Text)
    terminal_putentryat(218, 15 | 0 << 4, x, y); // ┌
    for (int i = 1; i < w - 1; i++) terminal_putentryat(196, 15 | 0 << 4, x + i, y); // ─
    terminal_putentryat(191, 15 | 0 << 4, x + w - 1, y); // ┐
    
    // Title text area
    for (int i = 1; i < w - 1; i++) terminal_putentryat(' ', 15 | 0 << 4, x + i, y);
    
    // Print title (starts at x + 1)
    if (title) {
        for (int i = 0; title[i] && (i < w - 4); i++) {
            terminal_putentryat(title[i], 15 | 0 << 4, x + 1 + i, y);
        }
    }

    // Close button (X)
    terminal_putentryat('x', 0 | 15 << 4, x + w - 2, y);

    // Window Body (Light Grey 7)
    for (int j = 1; j < h; j++) {
        terminal_putentryat(179, 8 | 7 << 4, x, y + j); // │
        for (int i = 1; i < w - 1; i++) terminal_putentryat(' ', 0 | 7 << 4, x + i, y + j);
        terminal_putentryat(179, 8 | 7 << 4, x + w - 1, y + j); // │
    }

    // Bottom border
    terminal_putentryat(192, 8 | 7 << 4, x, y + h); // └
    for (int i = 1; i < w - 1; i++) terminal_putentryat(196, 8 | 7 << 4, x + i, y + h); // ─
    terminal_putentryat(217, 8 | 7 << 4, x + w - 1, y + h); // ┘
}

// Special wrapper for backward compatibility with existing code
void draw_notepad(int x, int y, int w, int h) {
    draw_window(x, y, w, h, "NoteX");
}
