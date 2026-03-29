// multitask

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAX_WINDOWS 10
#define WIN_NOTEPAD 1
#define WIN_TTT 2

typedef struct {
    int type;
    int x, y;
    bool open;
    int cursor_x;
    int cursor_y;
} Window;

Window windows[MAX_WINDOWS];
int active_window = -1;
int window_count = 0;

int mtask_open(int type) {
    if (window_count >= MAX_WINDOWS) return -1;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].open) {
            windows[i].type = type;
            windows[i].x = 10 + (i * 4);
            windows[i].y = 5 + (i * 2);
            windows[i].open = true;
            windows[i].cursor_x = 1;
            windows[i].cursor_y = 1;
            window_count++;
            active_window = i;
            return i;
        }
    }
    return -1;
}

void mtask_close(int id) {
    if (id >= 0 && id < MAX_WINDOWS && windows[id].open) {
        windows[id].open = false;
        window_count--;
        if (active_window == id) {
            for (int i = MAX_WINDOWS - 1; i >= 0; i--) {
                if (windows[i].open) {
                    active_window = i;
                    break;
                }
            }
            if (active_window == -1) active_window = -1;
        }
    }
}

void mtask_focus(int id) {
    if (id >= 0 && id < MAX_WINDOWS && windows[id].open) {
        active_window = id;
    }
}

void mtask_reset() {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].open = false;
        windows[i].cursor_x = 0;
        windows[i].cursor_y = 0;
    }
    active_window = -1;
    window_count = 0;
}