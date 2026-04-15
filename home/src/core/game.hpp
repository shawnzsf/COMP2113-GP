#pragma once
#include "ftxui/dom/canvas.hpp"
#include "ftxui/component/event.hpp"
#include <vector>
#include <string>

struct Position {
    int x;
    int y;
};

struct Bullet {
    Position pos;
    bool active = true;
};

struct Enemy {
    Position pos;
    bool alive = true;
};

class Game {
public:
    Game();

    void Update();                    // Game logic (called every frame)
    void Draw(ftxui::Canvas& canvas); // Draw on canvas
    bool HandleEvent(const ftxui::Event& event);  // Returns true if event was consumed

    bool IsGameOver() const;
    int GetScore() const;
    int GetWave() const;
    
    static constexpr int WIDTH = 170;
    static constexpr int HEIGHT = 125;

private:
    // Game state
    Position player_pos;
    std::vector<Bullet> player_bullets;
    std::vector<Enemy> enemies;
    
    int score = 0;
    int frame_count = 0;
    int wave = 1;
    bool game_over = false;
    
    // Game constants
    static constexpr int PLAYER_MOVE_SPEED = 1;  // Smaller steps for smoother feel
    static constexpr int BULLET_MOVE_SPEED = 2;
    static constexpr int ENEMY_MOVE_SPEED = 1;
    static constexpr int ENEMY_MOVE_INTERVAL = 15;
    static constexpr int ENEMY_DESCENT_INTERVAL = 30;
    static constexpr int ENEMY_SPAWN_COUNT = 10;
    static constexpr int COLLISION_RADIUS = 1;
    static constexpr int POINTS_PER_ENEMY = 10;
    static constexpr int SHOOT_COOLDOWN = 6;  // Frames between shots
    int shoot_cooldown = 0;
    
    // Private methods
    void MoveEnemies();
    void MoveBullets();
    void CheckCollisions();
    void SpawnEnemies();
    void UpdateWave();
};
