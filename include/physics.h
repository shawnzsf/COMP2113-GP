#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>

// Tool types
enum class ToolType {
    NONE = 0,
    PLATE_LEFT,      // Sloped plate that redirects ball left
    PLATE_RIGHT,     // Sloped plate that redirects ball right
    BOUNCE_PAD,      // High bounce pad
    WALL,            // Solid wall
    NUM_TOOLS
};

// Position structure
struct Position {
    int x;
    int y;

    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Ball physics state
struct Ball {
    int x;
    int y;
    float vx;  // velocity x (float for smoother physics)
    float vy;  // velocity y

    Ball(int startX, int startY);
};

// Tool placement
struct Tool {
    Position pos;
    ToolType type;

    Tool(int x, int y, ToolType t) : pos(x, y), type(t) {}
};

class Physics {
public:
    Physics();

    // Get ball position (convert from fixed-point x10 to integer)
    int getBallX() const { return ball.x / 10; }
    int getBallY() const { return ball.y / 10; }

    // Check if ball reached exit
    bool hasReachedExit() const;

    // Check if ball is out of bounds
    bool isOutOfBounds() const;

    // Check collision with tools
    bool checkToolCollision(const Tool& tool) const;

    // Update ball physics
    void update(const std::vector<Tool>& tools);

    // Reset ball to start
    void reset();

private:
    Ball ball;
    int startX;
    int startY;

    // Apply gravity and move ball
    void applyGravity();

    // Handle collision response with tools
    void handleToolCollision(const Tool& tool);
};

#endif // PHYSICS_H
