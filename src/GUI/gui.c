#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// mouse bitmap
static unsigned char cursor_bmp[8] = {
    0b10000000, // *
    0b11000000, // **
    0b11100000, // ***
    0b11110000, // ****
    0b11111000, // *****
    0b11100000, // ****
    0b10100000, //   *
    0b00010000  //    *
};

extern void putpixel(int x, int y, uint32_t color);

void draw_cursor(int px, int py, int clicked) {
    uint32_t color = clicked ? 0x00FF4444 : 0x00FFFFFF;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (cursor_bmp[row] & (1 << (7 - col))) {
                putpixel(px + col, py + row, color);
            }
        }
    }
}
// global coordinates (back to 128x90 characters)
int mouse_x = 512, mouse_y = 360;
int old_mouse_x = 512, old_mouse_y = 360;
uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];

// kernel graphics and keyboard importation
extern void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
extern void terminal_initialize();
extern char scancode_to_ascii(uint8_t scancode);
extern void draw_notepad(int x, int y, int w, int h); // from WC_func.c
extern void draw_tictactoe(int x, int y); // from tictactoe.c
extern void ltos_switch_focus(int win_id); // from msche.c
extern int current_focus;
extern void mtask_reset();
extern int mtask_open(int type);
extern void mtask_close(int id);
extern void mtask_focus(int id);
extern void draw_eyes(int x, int y);
extern void draw_shell_window(int x, int y, int w, int h);
extern void gui_shell_execute(int x, int y, char* cmd);
extern int active_window;
extern int window_count;

typedef struct {
    int type; int x; int y;
    int open;
    int cursor_x; int cursor_y;
} Window;

extern Window windows[10];
#define WIN_NOTEPAD 1
#define WIN_TTT 2
#define WIN_EYES 3
#define WIN_SHELL 4

// helper to get window size based on type
static int get_win_w(int type) {
    if (type == WIN_NOTEPAD) return 25;
    if (type == WIN_TTT) return 13;
    if (type == WIN_EYES) return 16;
    if (type == WIN_SHELL) return 30;
    return 20;
}

static int get_win_h(int type) {
    if (type == WIN_NOTEPAD) return 10;
    if (type == WIN_TTT) return 8;
    if (type == WIN_EYES) return 10;
    if (type == WIN_SHELL) return 8;
    return 10;
}

// helper to draw icons on desktop
void draw_icon(int x, int y, char sym, uint8_t col, const char* name) {
    terminal_putentryat(sym, col | 8 << 4, x, y); // Icon on Grey bg
    for(int i=0; name[i]; i++) terminal_putentryat(name[i], 15 | 8 << 4, x-1+i, y+1);
}

// IO funcs
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// mouse funcs ps/2
void mouse_wait(uint8_t type) {
    if (type == 0) while (!(inb(0x64) & 1));
    else while (inb(0x64) & 2);
}

void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void mouse_init() {
    uint8_t status;
    while (inb(0x64) & 0x01) {
        inb(0x60);
    }
    mouse_wait(1);
    outb(0x64, 0xA8);
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = inb(0x60);
    status |= 0x02;
    status &= ~0x20;
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xFF);
    mouse_read(); mouse_read(); mouse_read();
    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();
}

void mouse_handler() {
    uint8_t status = inb(0x64);
    if (status & 0x01) {
        if (status & 0x20) {
            uint8_t data = inb(0x60);
            switch(mouse_cycle) {
                case 0:
                    if (!(data & 0x08)) return;
                    mouse_byte[0] = data;
                    mouse_cycle++;
                    break;
                case 1:
                    mouse_byte[1] = data;
                    mouse_cycle++;
                    break;
                case 2:
                    mouse_byte[2] = data;
                    mouse_x += (int8_t)mouse_byte[1];
                    mouse_y -= (int8_t)mouse_byte[2];
                    if (mouse_x < 0) mouse_x = 0;
                    if (mouse_y < 0) mouse_y = 0;
                    if (mouse_x > 1023) mouse_x = 1023;
                    if (mouse_y > 711) mouse_y = 711;
                    mouse_cycle = 0;
                    break;
            }
        } else {
            // we will not clean keyboard here anymore, 
            // the main loop will handle it non-blockingly
        }
    }
}

// disable mouse in shell
void mouse_disable() {
    mouse_write(0xF5);
    mouse_read();
}

extern void set_bga_resolution();

// RTC helpers
int rtc_updating() {
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

uint8_t get_rtc_register(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

int bcd_to_bin(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void draw_clock() {
    int hours, minutes, seconds;
    
    // Wait for RTC update to finish
    while (rtc_updating());

    hours   = bcd_to_bin(get_rtc_register(0x04));
    minutes = bcd_to_bin(get_rtc_register(0x02));
    seconds = bcd_to_bin(get_rtc_register(0x00));

    // Fuse hour (GMT-3 Brazil)
    hours = (hours + 21) % 24;

    uint8_t clock_color = 15 | 8 << 4;
    char time_str[9];
    time_str[0] = (hours / 10) + '0';
    time_str[1] = (hours % 10) + '0';
    time_str[2] = ':';
    time_str[3] = (minutes / 10) + '0';
    time_str[4] = (minutes % 10) + '0';
    time_str[5] = ':';
    time_str[6] = (seconds / 10) + '0';
    time_str[7] = (seconds % 10) + '0';
    time_str[8] = '\0';

    // Draw clock next to reset button (positions 114 to 121)
    for (int i = 0; i < 8; i++) {
        terminal_putentryat(time_str[i], clock_color, 114 + i, 88);
    }
}
void start_gui() {
    set_bga_resolution();
    mouse_init();
    uint8_t last_mouse_btn = 0;
    // paint the background of grey (8) with white text (15)
    uint8_t blue_bg = 8 << 4 | 15;
    for (int i = 0; i < 128 * 90; i++)
    terminal_putentryat(' ', blue_bg, i % 128, i / 128);

    bool is_dragging = false;
    int offset_x = 0, offset_y = 0;
    int dragging_win = -1;

    while(1) {
        // 0. DESKTOP ICONS
        draw_icon(5, 5, '#', 14, "NoteX");
        draw_icon(15, 5, 'X', 14, "TicTacToe");
        draw_icon(30, 5, 'O', 11, "Eyes");
        // shell icon, but i will add kernel's shell later
        // lets stay with a horizontal bar
        draw_icon(39, 5, '>', 14, "Shell");

        // 1. KEYBOARD (Non-blocking)
        if (inb(0x64) & 0x01) {
            uint8_t status = inb(0x64);
            if (!(status & 0x20)) {
                uint8_t scancode = inb(0x60);
                if (scancode == 0x01) { mouse_disable(); break; } // patched: conflit of q to exit to q on notex

                // Keyboard input goes to the active NoteX window
                if (active_window >= 0 && windows[active_window].open && windows[active_window].type == WIN_NOTEPAD) {
                    int *cx = &windows[active_window].cursor_x;
                    int *cy = &windows[active_window].cursor_y;
                    int wx = windows[active_window].x;
                    int wy = windows[active_window].y;
                    if (scancode == 0x0E) { // BACKSPACE
                        if (*cx > 1) { (*cx)--; terminal_putentryat(' ', 0 | 7 << 4, wx + *cx, wy + *cy); }
                    } else if (scancode == 0x1C) { // ENTER
                        if (*cy < 8) { (*cy)++; *cx = 1; }
                    } else {
                        char ch = scancode_to_ascii(scancode);
                        if (ch > 0 && *cx < 24) { terminal_putentryat(ch, 15 | 7 << 4, wx + *cx, wy + *cy); (*cx)++; }
                    }
                }

                // Keyboard input for Shell Window
                if (active_window >= 0 && windows[active_window].open && windows[active_window].type == WIN_SHELL) {
                    extern char gui_shell_buffer[64];
                    extern int gui_shell_idx;
                    int wx = windows[active_window].x;
                    int wy = windows[active_window].y;
                    char ch = scancode_to_ascii(scancode);

                    if (scancode == 0x1C) { // ENTER
                        gui_shell_buffer[gui_shell_idx] = '\0';
                        gui_shell_execute(wx, wy, gui_shell_buffer);
                        // Clear prompt visual for next command
                        for(int i=0; i<gui_shell_idx; i++) terminal_putentryat(' ', 15 | 0 << 4, wx + 8 + i, wy + 1);
                        gui_shell_idx = 0;
                    } else if (scancode == 0x0E) { // BACKSPACE
                        if (gui_shell_idx > 0) {
                            gui_shell_idx--;
                            terminal_putentryat(' ', 15 | 0 << 4, wx + 8 + gui_shell_idx, wy + 1);
                        }
                    } else if (ch > 0 && gui_shell_idx < 15) { 
                        terminal_putentryat(ch, 15 | 0 << 4, wx + 8 + gui_shell_idx, wy + 1);
                        gui_shell_buffer[gui_shell_idx++] = ch;
                    }
                }
            }
        }

        // 2. MOUSE
        mouse_handler();
        int mx = mouse_x / 8;
        int my = mouse_y / 8;
        draw_clock();
        // --- RESET BUTTON (bottom-right) ---
        terminal_putentryat('[', 12 | 8 << 4, 123, 88);
        terminal_putentryat('R', 12 | 8 << 4, 124, 88);
        terminal_putentryat('S', 12 | 8 << 4, 125, 88);
        terminal_putentryat('T', 12 | 8 << 4, 126, 88);
        terminal_putentryat(']', 12 | 8 << 4, 127, 88);

        // --- LOGIC: CLICK TO FOCUS & OPEN ---
        if ((mouse_byte[0] & 0x01) && !(last_mouse_btn & 0x01)) {

            // RESET button click
            if (mx >= 123 && mx <= 127 && my == 88) {
                mtask_reset();
                // repaint the entire backgroud
                for (int i = 0; i < 128 * 90; i++)
                    terminal_putentryat(' ', blue_bg, i % 128, i / 128);
                // redraw icons
                draw_icon(5, 5, '#', 14, "NoteX");
                draw_icon(15, 5, 2, 10, "TicTacToe");
                draw_icon(30, 5, 'O', 11, "Eyes");
            }

            // Click on NoteX Icon -> open new window
            if (mx >= 4 && mx <= 6 && my == 5)
                mtask_open(WIN_NOTEPAD);

            // Click on TicTacToe Icon -> open new window
            if (mx >= 14 && mx <= 16 && my == 5)
                mtask_open(WIN_TTT);

            // Click on Shell Icon -> open new window
            if (mx >= 38 && mx <= 40 && my == 5)
                mtask_open(WIN_SHELL);

            // Click on Eyes Icon -> open new window
            // lets patch this
            if (mx >= 30 && mx <= 31 && my == 5)
                mtask_open(WIN_EYES);

            // Click on any open window to focus it
            for (int i = 0; i < 10; i++) {
                if (!windows[i].open) continue;
                int w = get_win_w(windows[i].type);
                int h = get_win_h(windows[i].type);
                if (mx >= windows[i].x && mx < windows[i].x + w &&
                    my >= windows[i].y && my <= windows[i].y + h) {
                    mtask_focus(i);
                }
                // --- DETECT CLOSE BUTTON (X) ---
                int close_x = windows[i].x + w - 2;
                if (mx == close_x && my == windows[i].y) {
                    // Erase the window area before closing
                    int ww = w + 1;
                    int wh = h + 1;
                    for (int wy = 0; wy < wh; wy++)
                        for (int wx = 0; wx < ww; wx++)
                            terminal_putentryat(' ', blue_bg, windows[i].x + wx, windows[i].y + wy);
                    mtask_close(i);
                }
            }

            // Tic-Tac-Toe cell click
            if (active_window >= 0 && windows[active_window].open && windows[active_window].type == WIN_TTT) {
                extern int board[3][3];
                int tx = windows[active_window].x;
                int ty = windows[active_window].y;
                for (int r = 0; r < 3; r++) {
                    for (int c = 0; c < 3; c++) {
                        if (mx == tx + 2 + (c*4) && my == ty + 2 + (r*2))
                            if (board[r][c] == 0) board[r][c] = 1;
                    }
                }
            }
        }

        // --- DRAGGING ---
        if (mouse_byte[0] & 0x01) {
            if (!is_dragging && active_window >= 0 && windows[active_window].open) {
                int w = get_win_w(windows[active_window].type);
                if (my == windows[active_window].y && mx >= windows[active_window].x && mx < windows[active_window].x + w) {
                    is_dragging = true;
                    dragging_win = active_window;
                    offset_x = mx - windows[active_window].x;
                    offset_y = my - windows[active_window].y;
                }
            }
        } else {
            is_dragging = false;
            dragging_win = -1;
        }

        if (is_dragging && dragging_win >= 0) {
            if (windows[dragging_win].x != (mx - offset_x) || windows[dragging_win].y != (my - offset_y)) {
                for (int i = 0; i < 128 * 90; i++)
                    terminal_putentryat(' ', blue_bg, i % 128, i / 128);
                windows[dragging_win].x = mx - offset_x;
                windows[dragging_win].y = my - offset_y;
            }
        }
        // definition of mouse_clicked
        int mouse_clicked = (mouse_byte[0] & 0x01);
        // lets no remove nothing here
        if (mouse_x != old_mouse_x || mouse_y != old_mouse_y || (mouse_byte[0] & 0x07)) {
            int old_cx = old_mouse_x / 8;
            int old_cy = old_mouse_y / 8;
            for(int y=0; y<2; y++)
            for (int x=0; x<2; x++)
            terminal_putentryat(' ', blue_bg, old_cx + x, old_cy + y);
            for (int i = 0; i < 10; i++) {
                if (windows[i].open) {
                    if (windows[i].type == WIN_NOTEPAD)
                    draw_notepad(windows[i].x, windows[i].y, 25, 10);
                if (windows[i].type == WIN_TTT)
                draw_tictactoe(windows[i].x, windows[i].y);
                if (windows[i].type == WIN_EYES)
                draw_eyes(windows[i].x, windows[i].y);
                if (windows[i].type == WIN_SHELL)
                draw_shell_window(windows[i].x, windows[i].y, 30, 8);
                }
            }
            old_mouse_x = mouse_x;
            old_mouse_y = mouse_y;
            draw_cursor(mouse_x, mouse_y, mouse_clicked);
        }
        last_mouse_btn = mouse_byte[0]; 
        for (volatile int i = 0; i < 15000; i++);
    }
}