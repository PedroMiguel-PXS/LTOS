// here will be eye effect, same in xorg!

#include <stdint.h>

extern void putpixel(int x, int y, uint32_t color);
extern int mouse_x, mouse_y;
extern void draw_window(int x, int y, int w, int h, const char* title);

void draw_circle(int x0, int y0, int radius, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                putpixel(x0 + x, y0 + y, color);
            }
        }
    }
}

// draw eye structure, lets rename to draw_eye_base
void draw_eye_base(int x, int y) {
    for (int j = -20; j <= 20; j++) {
        for (int i = -15; i <= 15; i ++) {
            if (i * i + j * j <= 400) {
                putpixel(x + i, y + j, 0x00FFFFFF);
            }
        }
    }
}

void draw_eyes(int win_x, int win_y) {
    draw_window(win_x, win_y, 16, 10, "LT-Eyes");
    int px = win_x * 8;
    int py = win_y * 8;

    int eye1_x = px + 32;
    int eye1_y = py + 40;
    int eye2_x = px + 96;
    int eye2_y = py + 40;

    draw_eye_base(eye1_x, eye1_y);
    draw_eye_base(eye2_x, eye2_y);

    int centers_x[2] = {eye1_x, eye2_x};
    int centers_y[2] = {eye1_y, eye2_y};
    
    for (int k = 0; k < 2; k++) {
        int dx = mouse_x - centers_x[k];
        int dy = mouse_y - centers_y[k];
        
        int dist_sq = dx*dx + dy*dy;
        int limit = 10;
        if (dist_sq > limit * limit) {
            if (dx > limit) dx = limit;
            if (dx < -limit) dx = -limit;
            if (dy > limit) dy = limit;
            if (dy < -limit) dy = -limit;
        }

        draw_circle(centers_x[k] + dx, centers_y[k] + dy, 5, 0x000000);

    }
    
}