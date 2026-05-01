#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// we import the funcs that draw the window
extern void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
extern void draw_notepad(int x, int y, int w, int h);
// memory of tictactoe game
int board[3][3] = {{0,0,0}, {0,0,0}, {0,0,0}};
int ttt_turn = 1;
void ttt_reset() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = 0;
        }
    }
    ttt_turn = 1;
}
void draw_tictactoe(int x, int y) {
    draw_notepad(x, y, 13, 8);
    // write tic-tac
    terminal_putentryat('T', 15 | 0 << 4, x + 1, y);
    terminal_putentryat('I', 15 | 0 << 4, x + 2, y);
    terminal_putentryat('C', 15 | 0 << 4, x + 3, y);
    terminal_putentryat('-', 15 | 0 << 4, x + 4, y);
    terminal_putentryat('T', 15 | 0 << 4, x + 5, y);
    terminal_putentryat('A', 15 | 0 << 4, x + 6, y);
    terminal_putentryat('C', 15 | 0 << 4, x + 7, y);
    
    // draw the board
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            char s = ' ';
            if (board[i][j] == 1) s = 'X';
            if (board[i][j] == 2) s = 'O';
            // cells
            terminal_putentryat(s, 0 | 7 << 4, x + 2 + (j * 4), y + 2 + (i * 2));
            // vertical grades
            if (j < 2) terminal_putentryat('|', 0 | 7 << 4, x + 4 + (j * 4), y + 2 + (i * 2));
        
        } 
        // horizontals grades
        if (i < 2) {
            for (int k = 0; k < 9; k++) terminal_putentryat('-', 0 | 7 << 4, x + 2 + k, y + 3 + (i * 2));
        }
    }
}