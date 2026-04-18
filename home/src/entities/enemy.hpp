#pragma once
#include "ftxui/dom/canvas.hpp"
#include <vector>
#include <string>
#include "bullet.hpp"

enum class EnemyType {
    REGULAR,  // Basic enemy - 1 hit to kill, standard movement
    ELITE,    // Elite enemy - 3 hits to kill, random movement
    BOSS,     // Boss enemy - 10 hits to kill, fast random movement, can shoot
    CIRCLE_SHOOTER,  // Shoots an expanding circle of bullets every 240 frames
    MEGABOSS,  // Mega boss - 20 hits to kill, shoots bullets at player
    DROPSHIP   // Dropship boss - 120 hits to kill, spawns megabosses and bosses, shoots circles
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

    // For spawning enemies (dropship)
    std::vector<Enemy> spawned_enemies;

    // For dropship stream shooting
    int shoot_timer = 0;
    bool shooting_active = false;
    int shooting_timer = 0;

    Enemy() = default;
    Enemy(EnemyType t, Position p);

    // Methods for enemy behavior
    void Update(Position player_pos);  // Update enemy state (movement, shooting, etc.)
    void TakeDamage(int damage = 1);
    bool IsAlive() const { return alive && health > 0; }
    std::string GetSymbol() const;
    ftxui::Color GetColor() const;
    void Draw(ftxui::Canvas& canvas) const;

private:
    void UpdateRegular();
    void UpdateElite();
    void UpdateBoss();
    void UpdateCircleShooter();
    void UpdateMegaboss(Position player_pos);
    void UpdateDropship();
    void ShootBullet();
    void ShootCircle();
    void ShootAtPlayer(Position player_pos);
    void ShootMegabossSpread();
    void ShootStream();
};

// Enemy factory functions
Enemy CreateRegularEnemy(Position pos);
Enemy CreateEliteEnemy(Position pos);
Enemy CreateBossEnemy(Position pos);
Enemy CreateCircleShooterEnemy(Position pos);
Enemy CreateMegabossEnemy(Position pos);
Enemy CreateDropshipEnemy(Position pos);