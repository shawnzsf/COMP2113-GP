#ifndef INPUT_H
#define INPUT_H

#include <ncurses.h>
#include "physics.h"

class Input {
public:
    Input();
    ~Input();

    // Get arrow key direction, returns: 0=none, 1=left, 2=right, 3=up, 4=down
    int getArrowDirection();

    // Check for Enter key
    bool isEnterPressed();

    // Check for Escape key
    bool isEscapePressed();

    // Check for number keys (1-4 for tool selection)
    ToolType getToolSelection();

private:
};

#endif // INPUT_H
