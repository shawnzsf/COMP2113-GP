#pragma once
#include "ftxui/dom/canvas.hpp"
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

enum class EnemyType {
    REGULAR,  // Basic enemy - 1 hit to kill, standard movement
    ELITE,    // Elite enemy - 3 hits to kill, random movement
    BOSS      // Boss enemy - 10 hits to kill, fast random movement, can shoot
};

struct Enemy {
    Position pos;
    bool alive = true;
    EnemyType type = EnemyType::REGULAR;
    int health = 1;  // Current health
    int max_health = 1;  // Maximum health for this enemy type

    // Movement pattern variables
    int move_timer = 0;
    int move_direction_x = 1;
    int move_direction_y = 0;
    int random_seed = 0;  // For random movement patterns

    // Boss-specific: shooting capability
    int shoot_cooldown = 0;
    std::vector<Bullet> bullets;

    Enemy() = default;
    Enemy(EnemyType t, Position p);

    // Methods for enemy behavior
    void Update();  // Update enemy state (movement, shooting, etc.)
    void TakeDamage(int damage = 1);
    bool IsAlive() const { return alive && health > 0; }
    char GetSymbol() const;
    ftxui::Color GetColor() const;
    void Draw(ftxui::Canvas& canvas) const;

private:
    void UpdateRegular();
    void UpdateElite();
    void UpdateBoss();
    void ShootBullet();
};

// Enemy factory functions
Enemy CreateRegularEnemy(Position pos);
Enemy CreateEliteEnemy(Position pos);
Enemy CreateBossEnemy(Position pos);