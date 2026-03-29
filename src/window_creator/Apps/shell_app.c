// shell app

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

extern void terminal_putentryat(char c, uint8_t color, int x, int y);
extern void draw_window(int x, int y, int w, int h, const char* title);
extern int strcmp(const char* s1, const char* s2);

// imports to manipulate cursor
extern size_t terminal_column;
extern size_t terminal_row;
extern void shell_execute_command(char* input);
// local buffer of window
char gui_shell_buffer[64];
int gui_shell_idx = 0;

void draw_shell_window(int x, int y, int w, int h) {
    draw_window(x, y, w, h, "Shell");
    // draw a fixed prompt
    char* prompt = "RING0> ";
    for(int i=0; prompt[i]; i++) {
        terminal_putentryat(prompt[i], 10 | 0 << 4, x + 1 + i, y + 1);
    } 
}

// this function make the bridge between the GUI shell and the kernel shell
extern size_t terminal_margin_x;
void gui_shell_execute(int wx, int wy, char* cmd) {
    size_t old_x = terminal_column;
    size_t old_y = terminal_row;
    size_t old_margin = terminal_margin_x;
    terminal_margin_x = wx + 1;
    terminal_column = wx + 1;
    terminal_row = wy + 2;
    shell_execute_command(cmd);
    terminal_margin_x = old_margin;
    terminal_column = old_x;
    terminal_row = old_y;
}  
    

