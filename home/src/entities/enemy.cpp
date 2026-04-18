#include "enemy.hpp"
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <array>
#include <cmath>

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
            max_health = 15;
            health = 15;
            shoot_cooldown = 0;
            break;
        case EnemyType::CIRCLE_SHOOTER:
            max_health = 5;
            health = 5;
            shoot_cooldown = 0;
            break;
        case EnemyType::MEGABOSS:
            max_health = 60;
            health = 60;
            shoot_cooldown = 0;
            break;
        case EnemyType::DROPSHIP:
            max_health = 120;
            health = 120;
            shoot_timer = 100 + rand() % 200;  // Initial random delay
            break;
    }
}

void Enemy::Update(Position player_pos) {
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
        case EnemyType::CIRCLE_SHOOTER:
            UpdateCircleShooter();
            break;
        case EnemyType::MEGABOSS:
            UpdateMegaboss(player_pos);
            break;
        case EnemyType::DROPSHIP:
            UpdateDropship();
            break;
    }

    if (type == EnemyType::BOSS || type == EnemyType::CIRCLE_SHOOTER || type == EnemyType::MEGABOSS || type == EnemyType::DROPSHIP) {
        for (auto& bullet : bullets) {
            if (!bullet.active) continue;
            bullet.pos.x += bullet.dx;
            bullet.pos.y += bullet.dy;

            if (bullet.pos.y < 0 || bullet.pos.y > 125 || bullet.pos.x < 0 || bullet.pos.x > 170) {
                bullet.active = false;
            }
        }

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

void Enemy::UpdateCircleShooter() {
    move_timer++;

    if (shoot_cooldown > 0) {
        shoot_cooldown--;
    } else {
        if (move_timer % 120 == 0) {
            ShootCircle();
            shoot_cooldown = 120;
        }
    }
}

void Enemy::UpdateMegaboss(Position player_pos) {
    (void)player_pos;
    move_timer++;

    // Megaboss moves slowly and erratically
    if (move_timer % 16 == 0) {  // Change direction less frequently than regular boss
        int rand_val = (random_seed + move_timer) % 100;

        // Megaboss can move in 8 directions but more slowly
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

        // Megaboss moves slower than before
        int speed = 1;  // Slower movement
        pos.x += move_direction_x * speed;
        pos.y += move_direction_y * speed;

        // Keep within bounds (megaboss can go closer to edges)
        pos.x = std::max(10, std::min(160, pos.x));
        pos.y = std::max(10, std::min(80, pos.y));  // Megaboss stays in upper area
    }

    // Megaboss shooting logic - shoots every ~1.5 seconds
    if (shoot_cooldown > 0) {
        shoot_cooldown--;
    } else {
        ShootMegabossSpread();
        shoot_cooldown = 45;  // About 1.5 seconds at ~30 FPS
    }
}

void Enemy::UpdateDropship() {
    move_timer++;

    // Dropship moves like boss but faster
    if (move_timer % 8 == 0) {  // Change direction more frequently
        int rand_val = (random_seed + move_timer) % 100;

        // Dropship can move in 8 directions
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

        // Dropship moves faster
        int speed = 3;  // Faster movement
        pos.x += move_direction_x * speed;
        pos.y += move_direction_y * speed;

        // Keep within bounds (dropship stays in upper area)
        pos.x = std::max(15, std::min(155, pos.x));
        pos.y = std::max(15, std::min(70, pos.y));
    }

    // Stream shooting logic - increased fire rate
    if (shoot_timer > 0) {
        shoot_timer--;
    } else if (shooting_active) {
        // During active shooting, shoot every 4 frames (increased fire rate)
        if (move_timer % 4 == 0) {
            ShootStream();
        }
        shooting_timer--;
        if (shooting_timer <= 0) {
            shooting_active = false;
            shoot_timer = 100 + (random_seed + move_timer) % 150;  // Shorter random interval 100-249 frames
        }
    } else {
        // Start shooting
        shooting_active = true;
        shooting_timer = 120;  // Shoot for 120 frames (longer stream)
    }

    // Spawning logic - increased spawn rate, spawn every 400 frames
    if (move_timer % 400 == 0) {
        int rand_val = (random_seed + move_timer) % 100;
        if (rand_val < 50) {
            // Spawn megaboss
            Enemy megaboss = CreateMegabossEnemy({pos.x + 5, pos.y + 5});
            spawned_enemies.push_back(megaboss);
        } else {
            // Spawn boss
            Enemy boss = CreateBossEnemy({pos.x - 5, pos.y + 5});
            spawned_enemies.push_back(boss);
        }
    }
}

void Enemy::ShootBullet() {
    Bullet bullet;
    bullet.pos = {pos.x, pos.y + 2};  // Shoot from below the boss
    bullet.dx = 0;
    bullet.dy = 1;  // Move downwards
    bullet.active = true;
    bullets.push_back(bullet);
}

void Enemy::ShootCircle() {
    const int bullet_count = 32;
    for (int i = 0; i < bullet_count; ++i) {
        double angle = 2.0 * 3.14159265358979323846 * i / bullet_count;
        int dx = static_cast<int>(std::round(std::cos(angle)));
        int dy = static_cast<int>(std::round(std::sin(angle)));
        if (dx == 0 && dy == 0) {
            dx = 0; dy = 1;
        }

        Bullet bullet;
        bullet.pos = {pos.x, pos.y};
        bullet.dx = dx;
        bullet.dy = dy;
        bullet.active = true;
        bullets.push_back(bullet);
    }
}

void Enemy::ShootAtPlayer(Position player_pos) {
    // Calculate direction to player
    int dx = player_pos.x - pos.x;
    int dy = player_pos.y - pos.y;
    
    // Normalize the direction (simplified)
    if (dx != 0) dx = dx > 0 ? 1 : -1;
    if (dy != 0) dy = dy > 0 ? 1 : -1;
    
    // If both are 0, shoot down
    if (dx == 0 && dy == 0) {
        dy = 1;
    }

    Bullet bullet;
    bullet.pos = {pos.x, pos.y};
    bullet.dx = dx;
    bullet.dy = dy;
    bullet.length = 2;  // Twice as long as normal bullets
    bullet.active = true;
    bullets.push_back(bullet);
}

void Enemy::ShootMegabossSpread() {
    std::array<std::pair<int, int>, 3> directions = {{{-1, 0}, {0, 1}, {1, 0}}};
    for (const auto& dir : directions) {
        Bullet bullet;
        bullet.pos = {pos.x, pos.y};
        bullet.dx = dir.first;
        bullet.dy = dir.second;
        bullet.length = 3;  // Three units long
        bullet.active = true;
        bullets.push_back(bullet);
    }
}

void Enemy::ShootStream() {
    // Shoot two streams from opposite sides of the dropship - faster bullets
    Bullet left;
    left.pos = {pos.x - 4, pos.y + 3};  // From far left side
    left.dx = 0;
    left.dy = 2;  // Faster bullets
    left.active = true;
    bullets.push_back(left);

    Bullet right;
    right.pos = {pos.x + 4, pos.y + 3};  // From far right side
    right.dx = 0;
    right.dy = 2;  // Faster bullets
    right.active = true;
    bullets.push_back(right);
}

void Enemy::TakeDamage(int damage) {
    health -= damage;
    if (health <= 0) {
        alive = false;
    }
}

std::string Enemy::GetSymbol() const {
    switch (type) {
        case EnemyType::REGULAR:
            return "▼";
        case EnemyType::ELITE:
            return "▲";
        case EnemyType::BOSS: {
            std::string symbol = "▣";
            int lost = max_health - health;
            int symbols = 3 - (lost / (max_health / 3));
            if (symbols < 0) symbols = 0;
            std::string result;
            for (int i = 0; i < symbols; ++i) {
                result += symbol;
            }
            return result;

        }
        case EnemyType::CIRCLE_SHOOTER:
            if (health > max_health / 2) return "◉◉";
            else return "◉";
        case EnemyType::MEGABOSS: {
            int lost = max_health - health;
            int symbols = 6 - (lost / (max_health / 6));
            if (symbols < 0) symbols = 0;
            std::string result;
            for (int i = 0; i < symbols; ++i) {
                result += "🞚";
            }
            return result;
        }
        case EnemyType::DROPSHIP: {
            int lost = max_health - health;
            int state = lost / 20;  // 0 to 5
            int rows = 5 - state;  // Show up to 5 rows
            if (rows < 1) rows = 1;
            
            // Build the multi-line symbol directly
            std::string result;
            std::string line1 = "🞕🞖🞖🞖🞖🞕";
            std::string line2 = "🞕🞖🞖🞖🞖🞕";
            std::string line3 = "🞕🞖🞖🞖🞖🞕";
            std::string line4 = "🞕🞖🞖🞖🞖🞕";
            std::string line5 = "🞕🞖🞖🞖🞖🞕";
            
            if (rows >= 1) result += line1;
            if (rows >= 2) result += "\n" + line2;
            if (rows >= 3) result += "\n" + line3;
            if (rows >= 4) result += "\n" + line4;
            if (rows >= 5) result += "\n" + line5;
            
            return result;
        }
        default:
            return "▼";
    }
}

ftxui::Color Enemy::GetColor() const {
    switch (type) {
        case EnemyType::REGULAR:
            return ftxui::Color::Red;
        case EnemyType::ELITE:
            return ftxui::Color::Yellow;
        case EnemyType::BOSS:
            return ftxui::Color::Magenta;
        case EnemyType::CIRCLE_SHOOTER:
            return ftxui::Color::Blue;
        case EnemyType::MEGABOSS:
            return ftxui::Color::Red;  // Distinctive color for megaboss
        case EnemyType::DROPSHIP:
            return ftxui::Color::Cyan;  // Distinctive color for dropship
        default:
            return ftxui::Color::Red;
    }
}

void Enemy::Draw(ftxui::Canvas& canvas) const {
    if (!IsAlive()) return;

    std::string symbol = GetSymbol();
    if (type == EnemyType::DROPSHIP) {
        // Handle multi-line symbol
        size_t pos_start = 0;
        size_t pos_end = symbol.find('\n');
        int line = 0;
        while (pos_end != std::string::npos) {
            std::string line_symbol = symbol.substr(pos_start, pos_end - pos_start);
            canvas.DrawText(this->pos.x, this->pos.y + line, line_symbol, GetColor());
            pos_start = pos_end + 1;
            pos_end = symbol.find('\n', pos_start);
            line++;
        }
        // Last line
        std::string line_symbol = symbol.substr(pos_start);
        canvas.DrawText(this->pos.x, this->pos.y + line, line_symbol, GetColor());
    } else {
        canvas.DrawText(pos.x, pos.y, symbol, GetColor());
    }

    if (type == EnemyType::BOSS || type == EnemyType::CIRCLE_SHOOTER || type == EnemyType::MEGABOSS || type == EnemyType::DROPSHIP) {
        for (const auto& bullet : bullets) {
            if (!bullet.active) continue;
            std::string bullet_symbol = (type == EnemyType::DROPSHIP) ? "┃" : "*";
            canvas.DrawText(bullet.pos.x, bullet.pos.y, bullet_symbol, ftxui::Color::Red);
            if (bullet.length > 1) {
                canvas.DrawText(bullet.pos.x + bullet.dx, bullet.pos.y + bullet.dy, bullet_symbol, ftxui::Color::Red);
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

Enemy CreateCircleShooterEnemy(Position pos) {
    return Enemy(EnemyType::CIRCLE_SHOOTER, pos);
}

Enemy CreateMegabossEnemy(Position pos) {
    return Enemy(EnemyType::MEGABOSS, pos);
}

Enemy CreateDropshipEnemy(Position pos) {
    return Enemy(EnemyType::DROPSHIP, pos);
}