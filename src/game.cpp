#include "../include/game.h"
#include "../include/render.h"
#include <algorithm>
#include <random>
#include <ctime>

Game::Game() : phase(GamePhase::PLACEMENT), selectedTool(ToolType::PLATE_LEFT) {
    // Start with some tools available
}

int Game::getToolCount(ToolType type) const {
    // Count tools of specific type
    int count = 0;
    for (const auto& tool : placedTools) if (tool.type == type) count++;
    return count;
}

bool Game::placeTool(int x, int y, ToolType type) {
    // Validate position
    if (!canPlaceTool(x, y)) return false;
    // Don't place on start or end
    if ((x == START_X && y == START_Y) || (x == END_X && y == END_Y)) return false;
    // Add tool
    placedTools.push_back(Tool(x, y, type));
    return true;
}

bool Game::removeTool(int x, int y) {
    for (auto it = placedTools.begin(); it != placedTools.end(); ++it) {
        if (it->pos.x == x && it->pos.y == y) {
            placedTools.erase(it);
            return true;
        }
    }
    return false;
}

void Game::setSelectedTool(ToolType type) {
    selectedTool = type;
}

void Game::startSimulation() {
    if (phase == GamePhase::PLACEMENT) {
        phase = GamePhase::SIMULATION;
        physics.reset();
    }
}

void Game::reset() {
    phase = GamePhase::PLACEMENT;
    physics.reset();
    // Keep placed tools for retry
}

void Game::updatePhysics() {
    if (phase == GamePhase::SIMULATION) {
        physics.update(placedTools);
        checkGameEnd();
    }
}

void Game::checkGameEnd() {
    if (physics.hasReachedExit()) {
        phase = GamePhase::WIN;
    } else if (physics.isOutOfBounds()) {
        phase = GamePhase::LOSE;
    }
}

void Game::generateObstacles(int count) {
    obstacles.clear();

    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<int> xDist(2, GRID_WIDTH - 3);
    std::uniform_int_distribution<int> yDist(3, GRID_HEIGHT - 4);

    for (int i = 0; i < count; i++) {
        Position pos;
        bool valid = false;

        // Find valid position
        int attempts = 0;
        while (!valid && attempts < 100) {
            pos.x = xDist(rng);
            pos.y = yDist(rng);

            // Check not on start or end
            if ((pos.x == START_X && pos.y == START_Y) ||
                (pos.x == END_X && pos.y == END_Y)) {
                attempts++;
                continue;
            }

            // Check not overlapping with existing obstacles
            valid = true;
            for (const auto& obs : obstacles) {
                if (obs.x == pos.x && obs.y == pos.y) {
                    valid = false;
                    break;
                }
            }
            attempts++;
        }

        if (valid) {
            obstacles.push_back(pos);
        }
    }
}

bool Game::canPlaceTool(int x, int y) const {
    // Must be within bounds (not on border)
    if (x <= 0 || x >= GRID_WIDTH - 1 || y <= 0 || y >= GRID_HEIGHT - 1) {
        return false;
    }

    // Check not on start or end
    if ((x == START_X && y == START_Y) || (x == END_X && y == END_Y)) {
        return false;
    }

    // Check not on existing tool
    for (const auto& tool : placedTools) {
        if (tool.pos.x == x && tool.pos.y == y) {
            return false;
        }
    }

    // Check not on obstacle
    for (const auto& obs : obstacles) {
        if (obs.x == x && obs.y == y) {
            return false;
        }
    }

    return true;
}

const char* Game::getToolName(ToolType type) {
    switch (type) {
        case ToolType::PLATE_LEFT:   return "Plate (Left /)";
        case ToolType::PLATE_RIGHT:   return "Plate (Right \\)";
        case ToolType::BOUNCE_PAD:    return "Bounce Pad";
        case ToolType::WALL:          return "Wall";
        default:                      return "None";
    }
}

char Game::getToolChar(ToolType type) {
    switch (type) {
        case ToolType::PLATE_LEFT:   return '/';
        case ToolType::PLATE_RIGHT:   return '\\';
        case ToolType::BOUNCE_PAD:    return 'B';
        case ToolType::WALL:          return '#';
        default:                      return ' ';
    }
}
