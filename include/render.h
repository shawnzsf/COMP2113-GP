#ifndef RENDER_H
#define RENDER_H

#include <ncurses.h>

// Game constants - large map for smooth physics
const int GRID_WIDTH = 80;
const int GRID_HEIGHT = 40;
const int START_X = GRID_WIDTH / 2;
const int START_Y = 2;
const int END_X = GRID_WIDTH / 2;
const int END_Y = GRID_HEIGHT - 3;

class Render {
public:
    Render();
    ~Render();
    void init();// Initialize ncurses
    void shutdown();// Shutdown ncurses
    void clear(); // Clear screen
    void renderGrid(int ballX, int ballY, int cursorX, int cursorY,
                    const char* obstacles, const char* tools); // Render the game grid
    int getChar(); // Get character from terminal (non-blocking)

private:
    bool has_colors_flag;

    // Render a single cell
    void renderCell(int x, int y, char c, int colorPair);
};

#endif // RENDER_H
