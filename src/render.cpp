#include "../include/render.h"

Render::Render() : has_colors_flag(false) {
}

Render::~Render() {
    shutdown();
}

void Render::init() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        has_colors_flag = true;
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);     // Border
        init_pair(2, COLOR_GREEN, COLOR_BLACK);    // Start/End
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);   // Ball
        init_pair(4, COLOR_MAGENTA, COLOR_BLACK);  // Tools
        init_pair(5, COLOR_RED, COLOR_BLACK);      // Obstacles
        init_pair(6, COLOR_WHITE, COLOR_BLACK);    // Cursor
    }

    werase(stdscr);
    refresh();
}

void Render::shutdown() {
    curs_set(1);
    endwin();
}

void Render::clear() {
    // werase marks all cells as blank without flashing
    // clear() sends terminal clear command which causes flicker
    ::werase(stdscr);
}

int Render::getChar() {
    nodelay(stdscr, TRUE);
    int ch = getch();
    nodelay(stdscr, FALSE);
    return ch;
}

void Render::renderCell(int x, int y, char c, int colorPair) {
    attron(COLOR_PAIR(colorPair));
    mvaddch(y, x, c);
    attroff(COLOR_PAIR(colorPair));
}

void Render::renderGrid(int ballX, int ballY, int cursorX, int cursorY,
                        const char* obstacles, const char* tools) {
    // Clear screen (werase is flicker-free)
    ::werase(stdscr);

    // Render border
    for (int x = 0; x < GRID_WIDTH; x++) {
        renderCell(x, 0, '#', 1);
        renderCell(x, GRID_HEIGHT - 1, '#', 1);
    }
    for (int y = 0; y < GRID_HEIGHT; y++) {
        renderCell(0, y, '#', 1);
        renderCell(GRID_WIDTH - 1, y, '#', 1);
    }

    // Render Start
    renderCell(START_X, START_Y, 'S', 2);

    // Render End
    renderCell(END_X, END_Y, 'E', 2);

    // Render tools
    if (tools) {
        for (int y = 1; y < GRID_HEIGHT - 1; y++) {
            for (int x = 1; x < GRID_WIDTH - 1; x++) {
                char t = tools[y * GRID_WIDTH + x];
                if (t == 1) renderCell(x, y, '/', 4);
                else if (t == 2) renderCell(x, y, '\\', 4);
                else if (t == 3) renderCell(x, y, 'B', 4);
                else if (t == 4) renderCell(x, y, '#', 4);
            }
        }
    }

    // Render obstacles
    if (obstacles) {
        for (int y = 1; y < GRID_HEIGHT - 1; y++) {
            for (int x = 1; x < GRID_WIDTH - 1; x++) {
                if (obstacles[y * GRID_WIDTH + x] == 1) {
                    renderCell(x, y, 'X', 5);
                }
            }
        }
    }

    // Render cursor (placement indicator)
    if (cursorX > 0 && cursorX < GRID_WIDTH - 1 &&
        cursorY > 0 && cursorY < GRID_HEIGHT - 1) {
        renderCell(cursorX - 1, cursorY - 1, '+', 6);
        renderCell(cursorX, cursorY - 1, '-', 6);
        renderCell(cursorX + 1, cursorY - 1, '+', 6);
        renderCell(cursorX - 1, cursorY, '|', 6);
        renderCell(cursorX + 1, cursorY, '|', 6);
        renderCell(cursorX - 1, cursorY + 1, '+', 6);
        renderCell(cursorX, cursorY + 1, '-', 6);
        renderCell(cursorX + 1, cursorY + 1, '+', 6);
    }

    // Render ball
    if (ballX > 0 && ballX < GRID_WIDTH - 1 &&
        ballY > 0 && ballY < GRID_HEIGHT - 1) {
        renderCell(ballX, ballY, 'O', 3);
    }

    // Push to terminal
    refresh();
}
