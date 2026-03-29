#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../bitmaps/font8x8.h"

// i am brazilian, but i will do the comments in english
// Link external GUI module
extern void start_gui();
// keyboard driver
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
// translate to english
// function to force resolution to 1024x720
void set_bga_resolution() {
    outw(0x01CE, 4); outw(0x01CF, 0);      // Turn off the card to configure
    outw(0x01CE, 1); outw(0x01CF, 1024);   // Width
    outw(0x01CE, 2); outw(0x01CF, 720);    // Height
    outw(0x01CE, 3); outw(0x01CF, 32);     // 32 Bits per pixel (True Color!)
    outw(0x01CE, 4); outw(0x01CF, 0x01 | 0x40); // Turn on with LFB (Linear Framebuffer)
}

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8; 
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}
// here will be the comparer command
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

#define VGA_WIDTH 128
#define VGA_HEIGHT 90
#define VGA_MEMORY 0xB8000

void putpixel_vga(int x, int y, uint8_t color) {
    uint8_t* screen = (uint8_t*)VGA_MEMORY;
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        screen[y * VGA_WIDTH + x] = color;
    }
}

void vga_clear_screen(uint8_t color) {
    uint8_t* screen = (uint8_t*)VGA_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        screen[i] = color;
    }
}
// terminal margin
size_t terminal_margin_x = 0;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

void terminal_scroll() {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t current_index = y * VGA_WIDTH + x;
            const size_t next_index = (y + 1) * VGA_WIDTH + x;

            terminal_buffer[current_index] = terminal_buffer[next_index];
        }
    }
// clear just the last line(line 24)
for (size_t x = 0; x < VGA_WIDTH; x++) {
    const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(' ', terminal_color);
   }
}  

// Forward declaration: terminal_putentryat is defined later in this file
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

// --- LFB (Linear Framebuffer) - 1024x720 32bpp ---
uint32_t* framebuffer = (uint32_t*) 0xFD000000;

void vga_clear_lfb(uint32_t color) {
    for (int i = 0; i < 1024 * 720; i++) framebuffer[i] = color;
}

void putpixel(int x, int y, uint32_t color) {
    framebuffer[y * 1024 + x] = color;
}

// Converte cor VGA de 4 bits para 32-bit RGBA para o LFB
static uint32_t vga_to_rgb(uint8_t vga_color) {
    static uint32_t palette[16] = {
        0x00000000, // 0:  Black
        0x000000AA, // 1:  Blue
        0x0000AA00, // 2:  Green
        0x0000AAAA, // 3:  Cyan
        0x00AA0000, // 4:  Red
        0x00AA00AA, // 5:  Magenta
        0x00AA5500, // 6:  Brown
        0x00AAAAAA, // 7:  Light Gray
        0x00555555, // 8:  Dark Gray
        0x005555FF, // 9:  Light Blue
        0x0055FF55, // 10: Light Green
        0x0055FFFF, // 11: Light Cyan
        0x00FF5555, // 12: Light Red
        0x00FF55FF, // 13: Light Magenta
        0x00FFFF55, // 14: Yellow
        0x00FFFFFF  // 15: White
    };
    return palette[vga_color & 0x0F];
}

// Desenha um caractere na tela LFB usando a fonte 8x8
void draw_char(char c, int px, int py, uint32_t fg, uint32_t bg) {
    int idx = (unsigned char)c;
    if (idx < 0 || idx > 127) idx = ' ';
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (font8x8[idx][row] & (1 << (7 - col))) {
                putpixel(px + col, py + row, fg);
            } else {
                putpixel(px + col, py + row, bg);
            }
        }
    }
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    // Extrai cor do foreground (bits 0-3) e background (bits 4-7)
    uint8_t fg_idx = color & 0x0F;
    uint8_t bg_idx = (color >> 4) & 0x0F;
    uint32_t fg = vga_to_rgb(fg_idx);
    uint32_t bg = vga_to_rgb(bg_idx);
    // Cada "célula" de texto agora ocupa 8x8 pixels na tela LFB
    draw_char(c, (int)(x * 8), (int)(y * 8), fg, bg);
}
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = terminal_margin_x;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = VGA_HEIGHT - 1;
            terminal_scroll();
        }
        return;
    } else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        return;
    }
   terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
   if (++terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
        terminal_row = VGA_HEIGHT - 1;
        terminal_scroll();
       }
    }
} 
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* str) {
    terminal_write(str, strlen(str));
}
void terminal_print_logo(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("                 \n");
    terminal_writestring("            _.._\n");
    terminal_writestring("         .' .-'` \n");
    terminal_writestring("        /  /     \n");
    terminal_writestring("        |  |     \n");
    terminal_writestring("        \\  \\     \n");
    terminal_writestring("         '._'-._ \n");
    terminal_writestring("            ```  \n");
    terminal_writestring("       LTOS MOON EDITION\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}
// waiter function to wait for a key press
uint8_t keyboard_read() {
    while (!(inb(0x64) & 0x01));
    return inb(0x60);
}
// simple translator to scancode to ascii
char scancode_to_ascii(uint8_t scancode) {
    static char map[] = {
        // keyboard map
        // well, we will dont touch on this code, if you touch, you corrupt the kernel
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        ' ', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    if (scancode < sizeof(map)) return map[scancode];
    return 0;
}
// shell commands
void shell_execute_command(char* input) {
    if (strcmp(input, "help") == 0) {
        terminal_writestring("\n LTOS commands: help, clear, logo, about, gui, echo\n");
    } else if (strcmp(input, "clear") == 0) {
        terminal_initialize();
    } else if (strcmp(input, "logo") == 0) {
        terminal_print_logo();
    } else if (strcmp(input, "about") == 0) {
        terminal_writestring("\n LTOS is a legacy terminal OS made by Pedro Miguel\n");
    } else if (strcmp(input, "gui") == 0) {
        terminal_initialize();
        start_gui();
    // echo command
    } else if (strlen(input) > 5 && input[0]=='e' && input[1]=='c' && input[2]=='h' && input[3]=='o' && input[4]==' ') {
        terminal_writestring("\n ");
        terminal_writestring(input + 5); // prints everything after "echo "
        terminal_writestring("\n");
    } else if (strlen(input) > 0) {
        terminal_writestring("\n  THIS COMMAND DONT EXIST, TYPE HELP TO SEE THE COMMANDS: ");
        terminal_writestring(input);
        terminal_writestring("\n");
    }
    terminal_writestring("\nRING0> ");
    
}
// now, the SHELL!!
void shell_loop() {
    char buffer[256];
    int i = 0;
    terminal_writestring("\nRING0> ");
    while (1) {
        uint8_t scancode = keyboard_read();
        
        if (scancode == 0x1C) { // ENTER
            buffer[i] = '\0';
            shell_execute_command(buffer);
            i = 0;
        } else if (scancode == 0x0E) { // BACKSPACE
            if (i > 0) {
                i--;
                terminal_putchar('\b');
            }
        } else if (scancode < 0x80) {
            char c = scancode_to_ascii(scancode);
            if (c > 0 && i < 255) {
                terminal_putchar(c);
                buffer[i++] = c;
            }
        }
    }
}
void vga_set_mode_13h() {
    uint8_t g_320_200_256[] = {
        /* MISC */ 0x63,
        /* SEQ */ 0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */ 0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
                   0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF,
        /* GC */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
        /* AC */ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                 0x41, 0x00, 0x0F, 0x00, 0x00
    };

    // Write MISC
    outb(0x3C2, g_320_200_256[0]);

    // Write SEQ
    for (uint8_t i = 0; i < 5; i++) {
        outb(0x3C4, i);
        outb(0x3C5, g_320_200_256[1 + i]);
    }

    // Unlock CRTC (bit 7 of index 0x11)
    outb(0x3D4, 0x03);
    outb(0x3D5, inb(0x3D5) | 0x80);
    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & ~0x80);

    // Write CRTC
    for (uint8_t i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, g_320_200_256[6 + i]);
    }

    // Write GC
    for (uint8_t i = 0; i < 9; i++) {
        outb(0x3CE, i);
        outb(0x3CF, g_320_200_256[31 + i]);
    }

    // Write AC
    for (uint8_t i = 0; i < 21; i++) {
        inb(0x3DA); // Reset AC Flip-flop
        outb(0x3C0, i);
        outb(0x3C0, g_320_200_256[40 + i]);
    }

    inb(0x3DA);
    outb(0x3C0, 0x20); // Enable display
}

void kernel_main(void) {
    set_bga_resolution();
    terminal_initialize();
    terminal_print_logo();
    terminal_writestring("\n - system booted: ok\n");
    terminal_writestring("\n - loading modules...\n");
    terminal_writestring("\n - modules loaded: ok\n");
    terminal_writestring("\n- welcome to the shell!\n");
    terminal_writestring("\n- type 'help' for a list of commands\n");
    shell_loop();
}