#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include "physics.h"

// Game phases
enum class GamePhase {
    PLACEMENT,    // Player placing tools
    SIMULATION,   // Ball falling
    WIN,          // Ball reached exit
    LOSE          // Ball went out of bounds
};

class Game {
public:
    Game();

    // Get current phase
    GamePhase getPhase() const { return phase; }

    // Get player's tool count
    int getToolCount(ToolType type) const;

    // Get all placed tools
    const std::vector<Tool>& getTools() const { return placedTools; }

    // Place a tool at position
    bool placeTool(int x, int y, ToolType type);

    // Remove a tool at position
    bool removeTool(int x, int y);

    // Get selected tool type
    ToolType getSelectedTool() const { return selectedTool; }

    // Set selected tool type
    void setSelectedTool(ToolType type);

    // Start simulation
    void startSimulation();

    // Reset to placement phase
    void reset();

    // Update physics simulation
    void updatePhysics();

    // Get ball position
    int getBallX() const { return physics.getBallX(); }
    int getBallY() const { return physics.getBallY(); }

    // Check win/lose conditions
    void checkGameEnd();

    // Generate random obstacles (for future levels)
    void generateObstacles(int count);

    // Get tool name for display
    static const char* getToolName(ToolType type);
    static char getToolChar(ToolType type);

private:
    GamePhase phase;
    std::vector<Tool> placedTools;
    ToolType selectedTool;
    Physics physics;
    std::vector<Position> obstacles;  // Pre-placed obstacles

    // Check if position is valid for tool placement
    bool canPlaceTool(int x, int y) const;
};

#endif // GAME_H
