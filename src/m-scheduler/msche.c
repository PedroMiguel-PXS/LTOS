// mini scheduler
// super optimized for this OS

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
int current_focus = 0;

void ltos_switch_focus(int win_id) {
    if (current_focus == win_id) return;
    current_focus = win_id;
    // window panic: if change focus, clear the window to kill any artifacts
    // this is a simple way to do it
    uint8_t blue_bg = 8 << 4 | 15;
    for (int i = 0; i < 128 * 90; i++) {
        terminal_putentryat(' ', blue_bg, i % 128, i / 128);
    }
}