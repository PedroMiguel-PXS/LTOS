// here we gonna make a false loading screen on startup of the system

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// i am brazilian, but i will do the comments in english
extern void putpixel(int x, int y, uint32_t color);
extern void vga_clear_lfb(uint32_t color);
extern void terminal_writestring(const char* str);
// simple func to wait a little
void wait(int limit) {
    for (volatile int i = 0; i < limit * 10000; i++);
}
// start_floading
void start_floading() {
    vga_clear_lfb(0x00000000);
    int bar_x = 312;
    int bar_y = 400;
    int bar_w = 400;
    int bar_h = 20;
    // loop progress
    for (int p = 0; p <= 100; p++) {
        int current_w = (bar_w * p) / 100;
        for (int h = 0; h < bar_h; h++) {
            for (int w = 0; w < current_w; w++) {
                putpixel(bar_x + w, bar_y + h, 0x0000FF00);
            }
        }
        if (p == 10) terminal_writestring("Loading Kernel...");
        if (p == 25) terminal_writestring("Loading Drivers...");
        if (p == 50) terminal_writestring("Loading GUI...");
        if (p == 75) terminal_writestring("Starting LTOS...");
        if (p == 100) terminal_writestring("Done!");
        // its so fast!, lets put 500 to see the loading bar
        // perfect! 
        wait(500);
    }
}