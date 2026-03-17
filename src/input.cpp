#include "../include/input.h"

Input::Input() {
}

Input::~Input() {
}

int Input::getArrowDirection() {
    int ch = getch();
    if (ch == KEY_LEFT || ch == 'a' || ch == 'A') return 1;
    if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') return 2;
    if (ch == KEY_UP || ch == 'w' || ch == 'W') return 3;
    if (ch == KEY_DOWN || ch == 's' || ch == 'S') return 4;
    return 0;
}

bool Input::isEnterPressed() {
    int ch = getch();
    return ch == '\n' || ch == '\r' || ch == KEY_ENTER;
}

bool Input::isEscapePressed() {
    int ch = getch();
    return ch == 27;  // ESC key
}

ToolType Input::getToolSelection() {
    int ch = getch();
    if (ch == '1') return ToolType::PLATE_LEFT;
    if (ch == '2') return ToolType::PLATE_RIGHT;
    if (ch == '3') return ToolType::BOUNCE_PAD;
    if (ch == '4') return ToolType::WALL;
    return ToolType::NONE;
}
