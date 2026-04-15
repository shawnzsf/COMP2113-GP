#include "enemy.hpp"
#include <cstdlib>
#include <ctime>
#include <algorithm>

Enemy::Enemy(EnemyType t, Position p) : pos(p), type(t) {
    random_seed = rand() % 1000;  // Initialize with random seed

    switch (type) {
        case EnemyType::REGULAR:
            max_health = 1;
            health = 1;
            break;
        case EnemyType::ELITE:
            max_health = 3;
            health = 3;
            break;
        case EnemyType::BOSS:
            max_health = 10;
            health = 10;
            shoot_cooldown = 0;
            break;
    }
}

void Enemy::Update() {
    if (!IsAlive()) return;

    switch (type) {
        case EnemyType::REGULAR:
            UpdateRegular();
            break;
        case EnemyType::ELITE:
            UpdateElite();
            break;
        case EnemyType::BOSS:
            UpdateBoss();
            break;
    }

    // Update bullets for boss
    if (type == EnemyType::BOSS) {
        for (auto& bullet : bullets) {
            if (bullet.active) {
                bullet.pos.y += 1;  // Boss bullets move down
                if (bullet.pos.y > 125) {  // Off screen
                    bullet.active = false;
                }
            }
        }

        // Remove inactive bullets
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet& b) { return !b.active; }),
            bullets.end()
        );
    }
}

void Enemy::UpdateRegular() {
    // Simple left-right movement (formation movement will be handled by Game class)
    // This enemy type relies on the game's formation movement
    move_timer++;
}

void Enemy::UpdateElite() {
    move_timer++;

    // Elite enemies have more random movement patterns
    if (move_timer % 20 == 0) {  // Change direction every 20 frames
        // Random movement: sometimes move diagonally, sometimes change direction
        int rand_val = (random_seed + move_timer) % 100;

        if (rand_val < 30) {
            // 30% chance: move left
            move_direction_x = -1;
            move_direction_y = 0;
        } else if (rand_val < 60) {
            // 30% chance: move right
            move_direction_x = 1;
            move_direction_y = 0;
        } else if (rand_val < 80) {
            // 20% chance: move down-left
            move_direction_x = -1;
            move_direction_y = 1;
        } else {
            // 20% chance: move down-right
            move_direction_x = 1;
            move_direction_y = 1;
        }

        // Apply movement with some randomness in speed
        int speed = 1 + (rand_val % 2);  // Speed 1 or 2
        pos.x += move_direction_x * speed;
        pos.y += move_direction_y * speed;

        // Keep within bounds
        pos.x = std::max(2, std::min(168, pos.x));
        pos.y = std::max(2, std::min(120, pos.y));
    }
}

void Enemy::UpdateBoss() {
    move_timer++;

    // Boss has the most erratic movement
    if (move_timer % 8 == 0) {  // Change direction more frequently
        int rand_val = (random_seed + move_timer) % 100;

        // Boss can move in 8 directions
        if (rand_val < 12) {
            move_direction_x = -1; move_direction_y = -1;  // Up-left
        } else if (rand_val < 25) {
            move_direction_x = 0; move_direction_y = -1;   // Up
        } else if (rand_val < 37) {
            move_direction_x = 1; move_direction_y = -1;   // Up-right
        } else if (rand_val < 50) {
            move_direction_x = -1; move_direction_y = 0;   // Left
        } else if (rand_val < 62) {
            move_direction_x = 1; move_direction_y = 0;    // Right
        } else if (rand_val < 75) {
            move_direction_x = -1; move_direction_y = 1;   // Down-left
        } else if (rand_val < 87) {
            move_direction_x = 0; move_direction_y = 1;    // Down
        } else {
            move_direction_x = 1; move_direction_y = 1;    // Down-right
        }

        // Boss moves faster
        int speed = 2 + (rand_val % 2);  // Speed 2 or 3
        pos.x += move_direction_x * speed;
        pos.y += move_direction_y * speed;

        // Keep within bounds (boss can go closer to edges)
        pos.x = std::max(5, std::min(165, pos.x));
        pos.y = std::max(5, std::min(100, pos.y));  // Boss stays in upper area
    }

    // Boss shooting logic
    if (shoot_cooldown > 0) {
        shoot_cooldown--;
    } else {
        // Random chance to shoot
        int rand_val = (random_seed + move_timer) % 100;
        if (rand_val < 15) {  // 15% chance to shoot each frame when cooldown is ready
            ShootBullet();
            shoot_cooldown = 60;  // Cooldown between shots (60 frames)
        }
    }
}

void Enemy::ShootBullet() {
    Bullet bullet;
    bullet.pos = {pos.x, pos.y + 2};  // Shoot from below the boss
    bullet.active = true;
    bullets.push_back(bullet);
}

void Enemy::TakeDamage(int damage) {
    health -= damage;
    if (health <= 0) {
        alive = false;
    }
}

char Enemy::GetSymbol() const {
    switch (type) {
        case EnemyType::REGULAR:
            return 'v';
        case EnemyType::ELITE:
            // Show health status for elite enemies
            if (health == 3) return 'A';
            else if (health == 2) return '^';
            else return 'V';
        case EnemyType::BOSS:
            // Show health status for boss (different symbols)
            if (health > 7) return 'B';
            else if (health > 4) return 'b';
            else return 'X';
        default:
            return 'v';
    }
}

ftxui::Color Enemy::GetColor() const {
    switch (type) {
        case EnemyType::REGULAR:
            return ftxui::Color::Red;
        case EnemyType::ELITE:
            // Color changes with health
            if (health == 3) return ftxui::Color::Yellow;
            else if (health == 2) return ftxui::Color::Magenta;
            else return ftxui::Color::Red;
            break;
        case EnemyType::BOSS:
            // Boss has distinct colors
            if (health > 7) return ftxui::Color::Magenta;
            else if (health > 4) return ftxui::Color::Cyan;
            else return ftxui::Color::Red;
            break;
        default:
            return ftxui::Color::Red;
    }
}

void Enemy::Draw(ftxui::Canvas& canvas) const {
    if (!IsAlive()) return;

    canvas.DrawText(pos.x, pos.y, std::string(1, GetSymbol()), GetColor());

    // Draw boss bullets
    if (type == EnemyType::BOSS) {
        for (const auto& bullet : bullets) {
            if (bullet.active) {
                canvas.DrawText(bullet.pos.x, bullet.pos.y, "*", ftxui::Color::Red);
            }
        }
    }
}

// Factory functions
Enemy CreateRegularEnemy(Position pos) {
    return Enemy(EnemyType::REGULAR, pos);
}

Enemy CreateEliteEnemy(Position pos) {
    return Enemy(EnemyType::ELITE, pos);
}

Enemy CreateBossEnemy(Position pos) {
    return Enemy(EnemyType::BOSS, pos);
}