#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <ncurses.h>
#include "include/render.h"
#include "include/game.h"
#include "include/input.h"
#include "include/physics.h"

struct Cursor {
    int x;
    int y;
    Cursor() : x(GRID_WIDTH / 2), y(GRID_HEIGHT / 2) {}
};

void buildToolMap(char* toolMap, const std::vector<Tool>& tools) {
    memset(toolMap, 0, GRID_HEIGHT * GRID_WIDTH);
    for (const auto& tool : tools) {
        int idx = tool.pos.y * GRID_WIDTH + tool.pos.x;
        toolMap[idx] = static_cast<int>(tool.type);
    }
}

void renderGame(Render& render, Game& game, Cursor& cursor) {
    char toolMap[GRID_HEIGHT * GRID_WIDTH];
    buildToolMap(toolMap, game.getTools());

    int ballX = START_X, ballY = START_Y;
    if (game.getPhase() == GamePhase::SIMULATION ||
        game.getPhase() == GamePhase::WIN ||
        game.getPhase() == GamePhase::LOSE) {
        ballX = game.getBallX();
        ballY = game.getBallY();
    }

    // Pass cursor position for indicator
    int cursorX = -1, cursorY = -1;
    if (game.getPhase() == GamePhase::PLACEMENT) {
        cursorX = cursor.x;
        cursorY = cursor.y;
    }

    render.renderGrid(ballX, ballY, cursorX, cursorY, nullptr, toolMap);
    refresh();
}

int main() {
    Render render;
    render.init();

    Game game;
    Cursor cursor;

    clear();
    printw("====================================================\n");
    printw("            TERMINAL BALL DROP GAME\n");
    printw("====================================================\n");
    printw("Goal: Get the ball from Starting Position (S) to End Position (E) using tools\n");
    printw("\nPress ENTER to start...\n");
    refresh();

    while (true) {
        int ch = getch();
        if (ch == '\n' || ch == '\r') break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    renderGame(render, game, cursor);

    while (true) {
        nodelay(stdscr, TRUE);
        int ch = getch();
        nodelay(stdscr, FALSE);

        if (game.getPhase() == GamePhase::PLACEMENT) {
            if (ch != ERR) {
                if (ch == '1') game.setSelectedTool(ToolType::PLATE_LEFT);
                else if (ch == '2') game.setSelectedTool(ToolType::PLATE_RIGHT);
                else if (ch == '3') game.setSelectedTool(ToolType::BOUNCE_PAD);
                else if (ch == '4') game.setSelectedTool(ToolType::WALL);

                if (ch == KEY_LEFT || ch == 'a' || ch == 'A') if (cursor.x > 1) cursor.x--;
                if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') if (cursor.x < GRID_WIDTH - 2) cursor.x++;
                if (ch == KEY_UP || ch == 'w' || ch == 'W') if (cursor.y > 1) cursor.y--;
                if (ch == KEY_DOWN || ch == 's' || ch == 'S') if (cursor.y < GRID_HEIGHT - 2) cursor.y++;
                if (ch == ' ') {
                    bool removed = false; //when space is pressed, check if there's already a tool at cursor position. If yes, remove it. If no, place new tool.
                    for (const auto& tool : game.getTools()) {
                        if (tool.pos.x == cursor.x && tool.pos.y == cursor.y) {
                            game.removeTool(cursor.x, cursor.y);
                            removed = true;
                            break;
                        }
                    }
                    if (!removed) {
                        game.placeTool(cursor.x, cursor.y, game.getSelectedTool());
                    }
                }

                if (ch == '\n' || ch == '\r') {
                    game.startSimulation();
                }
            }
        }
        if (game.getPhase() == GamePhase::SIMULATION) {
            game.updatePhysics();

            if (ch == 27) { // ESC key to stop simulation and go back to placement
                game.reset();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // update physics every 50ms
        }
        renderGame(render, game, cursor);
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // update render every 20ms
    }

    render.shutdown();
    return 0;
}
