#include "../include/physics.h"
#include "../include/render.h"
#include <algorithm>
#include <cmath>

// Resolution factor: 10 = 10 sub-positions per grid cell
const int RESOLUTION = 10;

Ball::Ball(int startX, int startY) : x(startX * RESOLUTION), y(startY * RESOLUTION), vx(0), vy(0) {
}

Physics::Physics() : ball(START_X * RESOLUTION, START_Y * RESOLUTION), startX(START_X * RESOLUTION), startY(START_Y * RESOLUTION) {
}

bool Physics::hasReachedExit() const {
    // Check if ball is within 1 cell (RESOLUTION) of the exit position
    int targetX = END_X * RESOLUTION;
    int targetY = END_Y * RESOLUTION;
    return std::abs(ball.x - targetX) < RESOLUTION && std::abs(ball.y - targetY) < RESOLUTION;
}

bool Physics::isOutOfBounds() const {
    // Return true if ball is outside the grid boundaries (with some margin for resolution)
    return !(ball.x < GRID_WIDTH * RESOLUTION && ball.x > 0 &&
             ball.y < GRID_HEIGHT * RESOLUTION && ball.y > 0);
}

bool Physics::checkToolCollision(const Tool& tool) const {
    // toolX and toolY are the center of the tool in fixed-point coordinates
    // Check if ball is within 1 cell (RESOLUTION) of the tool's position
    int toolX = tool.pos.x * RESOLUTION;
    int toolY = tool.pos.y * RESOLUTION;
    return std::abs(ball.x - toolX) < RESOLUTION && std::abs(ball.y - toolY) < RESOLUTION;
}

void Physics::update(const std::vector<Tool>& tools) {
    // Check if ball is already colliding with a tool
    bool isColliding = false;
    for (const auto& tool : tools) {
        if (checkToolCollision(tool)) {
            isColliding = true;
            break;
        }
    }

    // Only apply gravity if NOT colliding
    if (!isColliding) {
        if (ball.vy > 0 || ball.vy < 0 || ball.y < (GRID_HEIGHT - 3) * RESOLUTION) {
            ball.vy += 1.0f;
        }
    }

    // Apply air resistance proportional to velocity
    float dragCoeff = 0.05f;
    if (std::abs(ball.vx) > 0.1f) ball.vx -= ball.vx * dragCoeff;
    if (std::abs(ball.vy) > 0.1f) ball.vy -= ball.vy * dragCoeff;

    // Terminal velocity
    if (ball.vy > 15) ball.vy = 15;
    if (ball.vy < -15) ball.vy = -15;
    if (ball.vx > 15) ball.vx = 15;
    if (ball.vx < -15) ball.vx = -15;

    // Calculate movement delta
    int dx = static_cast<int>(ball.vx);
    int dy = static_cast<int>(ball.vy);

    // Continuous Collision Detection, move in small steps to avoid tunneling through tools
    int steps = std::max(std::abs(dx), std::abs(dy));
    if (steps < 1) steps = 1;

    for (int i = 0; i < steps; i++) {
        ball.x += (dx > 0 ? 1 : (dx < 0 ? -1 : 0)); // Move 1 unit in x direction if vx is not zero
        ball.y += (dy > 0 ? 1 : (dy < 0 ? -1 : 0)); // Move 1 unit in y direction if vy is not zero

        for (const auto& tool : tools) {
            if (checkToolCollision(tool)) {
                handleToolCollision(tool); // Adjust velocity based on tool type
                dx = static_cast<int>(ball.vx); // Recalculate remaining movement after collision response
                dy = static_cast<int>(ball.vy);
            }
        }
    }

    // Stop if moving very slowly near bottom AND not colliding
    if (std::abs(ball.vx) < 0.5f && std::abs(ball.vy) < 0.5f) {
        bool stillColliding = false;
        for (const auto& tool : tools) {
            if (checkToolCollision(tool)) {
                stillColliding = true;
                break;
            }
        }
        if (!stillColliding && ball.y > (GRID_HEIGHT - 5) * RESOLUTION) {
            ball.vx = 0;
            ball.vy = 0;
        }
    }
}

void Physics::handleToolCollision(const Tool& tool) {
    float speed = std::sqrt(ball.vx * ball.vx + ball.vy * ball.vy);
    if (speed < 2.0f) return;

    float energyLoss = 0.7f;

    switch (tool.type) {
        case ToolType::PLATE_LEFT:
            ball.vy = -std::abs(ball.vy) * energyLoss - 3.0f;
            ball.vx = -std::abs(ball.vx) * energyLoss - 2.0f;
            if (ball.vx > 0) ball.vx = -ball.vx;
            break;
        case ToolType::PLATE_RIGHT:
            ball.vy = -std::abs(ball.vy) * energyLoss - 3.0f;
            ball.vx = std::abs(ball.vx) * energyLoss + 2.0f;
            if (ball.vx < 0) ball.vx = -ball.vx;
            break;
        case ToolType::BOUNCE_PAD:
            ball.vy = -std::abs(ball.vy) * 1.2f - 5.0f;
            ball.vx = ball.vx * 0.8f;
            break;
        case ToolType::WALL:
            ball.vx = -ball.vx * energyLoss;
            ball.vy = -ball.vy * energyLoss;
            break;
        default:
            break;
    }
}

void Physics::reset() {
    ball.x = startX;
    ball.y = startY;
    ball.vx = 0;
    ball.vy = 0;
}
