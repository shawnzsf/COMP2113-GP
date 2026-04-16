#pragma once
#include "ftxui/dom/canvas.hpp"
#include "ftxui/component/event.hpp"
#include "../entities/enemy.hpp"
#include "../entities/player.hpp"
#include <vector>
#include <string>

enum class WeaponType {
    BASIC,
    DUAL,
    TRI,
    EXPLOSIVE
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
    bool CanAfford(int amount) const;
    bool BuyWeapon(WeaponType type, int cost);
    bool BuyShield(int cost);
    int GetCash() const;
    WeaponType GetWeaponType() const;
    bool HasShield() const;
    
    static constexpr int WIDTH = 170;
    static constexpr int HEIGHT = 125;

private:
    // Game state
    Player player;
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
    static constexpr int POINTS_REGULAR_ENEMY = 10;
    static constexpr int POINTS_ELITE_ENEMY = 30;
    static constexpr int POINTS_BOSS_ENEMY = 100;
    static constexpr int SHOOT_COOLDOWN = 6;  // Frames between shots
    static constexpr int INPUT_HOLD_FRAMES = 6;  // Keep a direction active briefly after the last event
    int shoot_cooldown = 0;

    // Input state
    bool move_left = false;
    bool move_right = false;
    bool move_up = false;
    bool move_down = false;
    int move_left_timer = 0;
    int move_right_timer = 0;
    int move_up_timer = 0;
    int move_down_timer = 0;
    bool shoot_requested = false;

    // Player upgrades and currency
    int cash = 0;
    WeaponType weapon_type = WeaponType::BASIC;

    // Private methods
    void MoveEnemies();
    void MoveBullets();
    void CheckCollisions();
    void SpawnEnemies();
    void UpdateWave();
    void FireWeapon();
};
