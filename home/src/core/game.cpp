#include "game.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

Game::Game() {
    srand(time(nullptr));  // Initialize random seed
    player.SetBounds(WIDTH, HEIGHT);
    player.SetPosition(WIDTH / 2, HEIGHT - 3);
    SpawnEnemies();
}

void Game::SpawnEnemies() {
    enemies.clear();
    // Spawn enemies in a grid pattern, with difficulty based on wave
    int enemy_count = ENEMY_SPAWN_COUNT + (wave - 1) * 2;  // More enemies per wave
    int start_x = (WIDTH - (enemy_count * 6)) / 2;  // Center the formation

    // Enemy type probabilities (higher wave = more elite/boss enemies)
    int megaboss_chance = std::min(2 + wave / 5, 5);     // 2% base, up to 5% (very rare)
    int boss_chance = std::min(5 + wave, 15);            // 5% base, up to 15%
    int circle_shooter_chance = std::min(8 + wave, 20);  // 8% base, up to 20%
    int elite_chance = std::min(10 + wave * 2, 30);      // 10% base, up to 30%
    // Regular enemies get the remainder

    for (int i = 0; i < enemy_count && i < 20; ++i) {  // Cap at 20 enemies
        Position pos = {start_x + (i * 6), 3 + (i / 5) * 3};  // Multiple rows

        // Determine enemy type based on random chance
        int rand_val = rand() % 100;

        Enemy enemy;
        if (rand_val < megaboss_chance) {
            // Megaboss enemy (rarest)
            enemy = CreateMegabossEnemy(pos);
        } else if (rand_val < megaboss_chance + boss_chance) {
            // Boss enemy (least frequent)
            enemy = CreateBossEnemy(pos);
        } else if (rand_val < megaboss_chance + boss_chance + circle_shooter_chance) {
            // Circle shooter enemy (moderate frequency)
            enemy = CreateCircleShooterEnemy(pos);
        } else if (rand_val < megaboss_chance + boss_chance + circle_shooter_chance + elite_chance) {
            // Elite enemy (medium frequency)
            enemy = CreateEliteEnemy(pos);
        } else {
            // Regular enemy (most frequent)
            enemy = CreateRegularEnemy(pos);
        }

        enemies.push_back(enemy);
    }
}

bool Game::HandleEvent(const ftxui::Event& event) {
    if (game_over) {
        return event == ftxui::Event::Character('q') ||
               event == ftxui::Event::Character('Q');
    }

    bool handled = false;

    // Handle movement - allow multiple directions simultaneously
    if (event == ftxui::Event::ArrowLeft || event == ftxui::Event::Character('a') ||
        event == ftxui::Event::Character('A')) {
        move_left = true;
        move_left_timer = INPUT_HOLD_FRAMES;
        handled = true;
    }

    if (event == ftxui::Event::ArrowRight || event == ftxui::Event::Character('d') ||
        event == ftxui::Event::Character('D')) {
        move_right = true;
        move_right_timer = INPUT_HOLD_FRAMES;
        handled = true;
    }

    if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('w') ||
        event == ftxui::Event::Character('W')) {
        move_up = true;
        move_up_timer = INPUT_HOLD_FRAMES;
        handled = true;
    }

    if (event == ftxui::Event::ArrowDown || event == ftxui::Event::Character('s') ||
        event == ftxui::Event::Character('S')) {
        move_down = true;
        move_down_timer = INPUT_HOLD_FRAMES;
        handled = true;
    }

    // Handle shooting with cooldown
    if (event == ftxui::Event::Character(' ')) {
        shoot_hold_timer = SHOOT_HOLD_FRAMES;
        handled = true;
    }

    return handled;
}

void Game::Update() {
    if (game_over) return;

    frame_count++;

    // Update shoot cooldown and shooting hold timer
    if (shoot_cooldown > 0) {
        shoot_cooldown--;
    }
    if (shoot_hold_timer > 0) {
        shoot_hold_timer--;
    }

    // Apply input state for movement
    if (move_left_timer > 0) {
        player.MoveLeft();
        move_left_timer--;
        move_left = move_left_timer > 0;
    }
    if (move_right_timer > 0) {
        player.MoveRight();
        move_right_timer--;
        move_right = move_right_timer > 0;
    }
    if (move_up_timer > 0) {
        player.MoveUp();
        move_up_timer--;
        move_up = move_up_timer > 0;
    }
    if (move_down_timer > 0) {
        player.MoveDown();
        move_down_timer--;
        move_down = move_down_timer > 0;
    }

    // Apply shooting state independently of movement
    if (shoot_hold_timer > 0 && shoot_cooldown <= 0) {
        FireWeapon();
        shoot_cooldown = SHOOT_COOLDOWN;
    }

    // Update game logic
    MoveBullets();
    MoveEnemies();
    CheckCollisions();
    UpdateWave();

    // Enemy descent (independent of movement)
    if (frame_count % ENEMY_DESCENT_INTERVAL == 0) {
        for (auto& e : enemies) {
            if (e.alive) e.pos.y++;
        }
    }

    // Check game over conditions - enemies reach near player level
    for (const auto& e : enemies) {
        if (e.alive && e.pos.y >= HEIGHT - 15) {  // More lenient condition
            game_over = true;
            break;
        }
    }
}

void Game::MoveBullets() {
    std::vector<Bullet> spawned_fragments;

    for (auto& b : player_bullets) {
        if (!b.active) continue;

        b.pos.x += b.dx;
        b.pos.y += b.dy;

        // Explosive bullets now explode on impact, not on timer
        // Remove the timer-based explosion logic

        if (b.type == BulletType::FRAGMENT) {
            b.lifetime--;
            if (b.lifetime <= 0) {
                b.active = false;
            }
        }

        if (b.pos.y < 1 || b.pos.y >= HEIGHT - 1 || b.pos.x < 1 || b.pos.x >= WIDTH - 1) {
            b.active = false;
        }
    }

    // Add explosion fragments after the main pass
    player_bullets.insert(player_bullets.end(), spawned_fragments.begin(), spawned_fragments.end());

    // Remove inactive bullets to save memory
    player_bullets.erase(
        std::remove_if(player_bullets.begin(), player_bullets.end(),
            [](const Bullet& b) { return !b.active; }),
        player_bullets.end()
    );
}

void Game::FireWeapon() {
    Position player_pos = player.GetPosition();

    if (weapon_type == WeaponType::BASIC) {
        Bullet b;
        b.active = true;
        b.pos = {player_pos.x + 1, player_pos.y - 1};
        b.dx = 0;
        b.dy = -1;
        b.type = BulletType::NORMAL;
        player_bullets.push_back(b);
        return;
    }

    if (weapon_type == WeaponType::DUAL) {
        Bullet left;
        left.active = true;
        left.pos = {player_pos.x, player_pos.y - 1};
        left.dx = -1;
        left.dy = -1;
        left.type = BulletType::NORMAL;

        Bullet right;
        right.active = true;
        right.pos = {player_pos.x + 2, player_pos.y - 1};
        right.dx = 1;
        right.dy = -1;
        right.type = BulletType::NORMAL;

        player_bullets.push_back(left);
        player_bullets.push_back(right);
        return;
    }

    if (weapon_type == WeaponType::TRI) {
        Bullet center;
        center.active = true;
        center.pos = {player_pos.x + 1, player_pos.y - 1};
        center.dx = 0;
        center.dy = -1;
        center.type = BulletType::NORMAL;

        Bullet left;
        left.active = true;
        left.pos = {player_pos.x, player_pos.y - 1};
        left.dx = -1;
        left.dy = -1;
        left.type = BulletType::NORMAL;

        Bullet right;
        right.active = true;
        right.pos = {player_pos.x + 2, player_pos.y - 1};
        right.dx = 1;
        right.dy = -1;
        right.type = BulletType::NORMAL;

        player_bullets.push_back(center);
        player_bullets.push_back(left);
        player_bullets.push_back(right);
        return;
    }

    if (weapon_type == WeaponType::EXPLOSIVE) {
        Bullet orb;
        orb.active = true;
        orb.pos = {player_pos.x + 1, player_pos.y - 1};
        orb.dx = 0;
        orb.dy = -1;
        orb.type = BulletType::EXPLOSIVE;
        player_bullets.push_back(orb);
        return;
    }
}

void Game::MoveEnemies() {
    // Handle formation movement for regular enemies
    static int direction = 1;

    if (frame_count % ENEMY_MOVE_INTERVAL == 0) {
        bool edge_reached = false;

        // Check if any alive formation enemy reached the edge
        for (const auto& e : enemies) {
            if (e.IsAlive() && (e.type == EnemyType::REGULAR || e.type == EnemyType::CIRCLE_SHOOTER)) {
                if ((direction > 0 && e.pos.x >= WIDTH - 6) ||
                    (direction < 0 && e.pos.x <= 4)) {
                    edge_reached = true;
                    break;
                }
            }
        }

        if (edge_reached) {
            // Reverse direction and descend slightly for regular formation enemies
            direction = -direction;
            for (auto& e : enemies) {
                if (e.IsAlive() && (e.type == EnemyType::REGULAR || e.type == EnemyType::CIRCLE_SHOOTER)) {
                    e.pos.y += 2;  // Descend when hitting edge
                }
            }
        } else {
            // Move formation enemies in current direction
            for (auto& e : enemies) {
                if (e.IsAlive() && (e.type == EnemyType::REGULAR || e.type == EnemyType::CIRCLE_SHOOTER)) {
                    e.pos.x += direction * ENEMY_MOVE_SPEED;
                }
            }
        }
    }

    // Update all enemies (this handles individual movement for elite/boss, and any other updates)
    Position player_pos = player.GetPosition();
    for (auto& e : enemies) {
        e.Update(player_pos);
    }
}

void Game::CheckCollisions() {
    for (auto& b : player_bullets) {
        if (!b.active) continue;

        for (auto& e : enemies) {
            float collision_radius = 1.0f;
            if (e.type == EnemyType::BOSS) collision_radius = 2.0f;
            else if (e.type == EnemyType::MEGABOSS) collision_radius = 3.0f;
            
            if (b.type == BulletType::EXPLOSIVE) collision_radius *= 2.0f;
            else if (b.type == BulletType::NORMAL) collision_radius *= 1.5f;
            
            if (e.IsAlive() &&
                std::abs(static_cast<float>(b.pos.x - e.pos.x)) <= collision_radius &&
                std::abs(static_cast<float>(b.pos.y - e.pos.y)) <= collision_radius) {

                // Deal damage to enemy
                e.TakeDamage(1);

                // Handle explosive bullets - they explode on impact
                if (b.type == BulletType::EXPLOSIVE) {
                    // Spawn explosion fragments in 8 directions
                    const std::vector<std::pair<int, int>> directions = {
                        {-1, -1}, {0, -1}, {1, -1},
                        {-1, 0},           {1, 0},
                        {-1, 1},  {0, 1},  {1, 1}
                    };

                    for (const auto& dir : directions) {
                        Bullet fragment;
                        fragment.type = BulletType::FRAGMENT;
                        fragment.active = true;
                        fragment.pos = b.pos;
                        fragment.dx = dir.first;
                        fragment.dy = dir.second;
                        fragment.lifetime = 30; // lasts about half a second
                        player_bullets.push_back(fragment);
                    }
                }

                b.active = false;

                // Award points and cash only if enemy is destroyed
                if (!e.IsAlive()) {
                    switch (e.type) {
                        case EnemyType::REGULAR:
                            score += POINTS_REGULAR_ENEMY;
                            cash += 5;
                            break;
                        case EnemyType::ELITE:
                            score += POINTS_ELITE_ENEMY;
                            cash += 15;
                            break;
                        case EnemyType::BOSS:
                            score += POINTS_BOSS_ENEMY;
                            cash += 50;
                            break;
                        case EnemyType::CIRCLE_SHOOTER:
                            score += 60;
                            cash += 15;
                            break;
                        case EnemyType::MEGABOSS:
                            score += 200;
                            cash += 100;
                            break;
                    }
                }

                break;  // Bullet only hits one enemy
            }
        }
    }

    // Check collisions between enemy bullets and player
    for (auto& e : enemies) {
        if (e.type == EnemyType::BOSS || e.type == EnemyType::CIRCLE_SHOOTER || e.type == EnemyType::MEGABOSS) {
            for (const auto& enemy_bullet : e.bullets) {
                if (!enemy_bullet.active) continue;

                Position player_pos = player.GetPosition();
                float bullet_radius = (enemy_bullet.type == BulletType::NORMAL) ? 1.5f : 1.0f;
                if (std::abs(static_cast<float>(enemy_bullet.pos.x - player_pos.x)) <= bullet_radius &&
                    std::abs(static_cast<float>(enemy_bullet.pos.y - player_pos.y)) <= bullet_radius) {

                    player.TakeDamage(1);

                    if (!player.IsAlive()) {
                        game_over = true;
                        return;
                    }
                }
            }
        }
    }
}

void Game::UpdateWave() {
    // Check if all enemies are defeated
    bool all_defeated = true;
    for (const auto& e : enemies) {
        if (e.IsAlive()) {
            all_defeated = false;
            break;
        }
    }

    if (all_defeated && !enemies.empty()) {
        wave++;
        SpawnEnemies();
    }
}

void Game::Draw(ftxui::Canvas& canvas) {
    // Draw border
    // for (int x = 0; x < WIDTH; ++x) {
    //     canvas.DrawText(x, 0, "─", ftxui::Color::White);
    //     canvas.DrawText(x, HEIGHT - 1, "─", ftxui::Color::White);
    // }
    // for (int y = 1; y < HEIGHT - 1; ++y) {
    //     canvas.DrawText(0, y, "│", ftxui::Color::White);
    //     canvas.DrawText(WIDTH - 1, y, "│", ftxui::Color::White);
    // }
    
    // // Draw corners
    // canvas.DrawText(0, 0, "┌", ftxui::Color::White);
    // canvas.DrawText(WIDTH - 1, 0, "┐", ftxui::Color::White);
    // canvas.DrawText(0, HEIGHT - 1, "└", ftxui::Color::White);
    // canvas.DrawText(WIDTH - 1, HEIGHT - 1, "┘", ftxui::Color::White);

    // Draw Player
    player.Draw(canvas);

    // Draw Enemies
    for (const auto& e : enemies) {
        e.Draw(canvas);
    }

    // Draw Player Bullets
    for (const auto& b : player_bullets) {
        if (b.active) {
            std::string symbol = (b.type == BulletType::EXPLOSIVE) ? "@" : "•";
            if (b.type == BulletType::FRAGMENT) symbol = ".";
            canvas.DrawText(b.pos.x, b.pos.y, symbol, ftxui::Color::Yellow);
        }
    }

    // // Draw HUD (Score, Cash, and Wave)
    // std::string score_text = "Score: " + std::to_string(score);
    // std::string cash_text = "Cash: $" + std::to_string(cash);
    // std::string wave_text = "Wave: " + std::to_string(wave);
    // std::string enemies_text = "Enemies: " + std::to_string(
    //     std::count_if(enemies.begin(), enemies.end(), 
    //         [](const Enemy& e) { return e.IsAlive(); })
    // );
    // std::string hp_text = "HP: " + std::to_string(player.GetHealth());
    
    // canvas.DrawText(2, 1, score_text, ftxui::Color::Yellow);
    // canvas.DrawText(2, 5, cash_text, ftxui::Color::Green);
    // canvas.DrawText(WIDTH / 2 - 8, 1, wave_text, ftxui::Color::Cyan);
    // canvas.DrawText(WIDTH - 35, 1, enemies_text, ftxui::Color::Magenta);
    // canvas.DrawText(WIDTH - 12, 1, hp_text, ftxui::Color::Red);

    // Draw game over message
    if (game_over) {
        canvas.DrawText(WIDTH / 2 - 5, HEIGHT / 2, "GAME OVER", ftxui::Color::Red);
        canvas.DrawText(WIDTH / 2 - 8, HEIGHT / 2 + 4, "Final Score: " + std::to_string(score), ftxui::Color::White);
        canvas.DrawText(WIDTH / 2 - 7, HEIGHT / 2 + 8, "Press Q to quit", ftxui::Color::Yellow);
    }
}

bool Game::IsGameOver() const {
    return game_over;
}

int Game::GetScore() const {
    return score;
}

int Game::GetWave() const {
    return wave;
}

bool Game::CanAfford(int amount) const {
    return cash >= amount;
}

bool Game::BuyWeapon(WeaponType type, int cost) {
    if (type == weapon_type || !CanAfford(cost)) {
        return false;
    }
    cash -= cost;
    weapon_type = type;
    return true;
}

bool Game::BuyShield(int cost) {
    if (player.HasShield() || !CanAfford(cost)) {
        return false;
    }
    cash -= cost;
    player.ActivateShield();
    return true;
}

int Game::GetCash() const {
    return cash;
}

WeaponType Game::GetWeaponType() const {
    return weapon_type;
}

bool Game::HasShield() const {
    return player.HasShield();
}

int Game::GetShootCooldown() const {
    return shoot_cooldown;
}

int Game::GetMaxShootCooldown() const {
    return SHOOT_COOLDOWN;
}

bool Game::HasRapidFire() const {
    return has_rapid_fire;
}

bool Game::HasTimeSlow() const {
    return has_time_slow;
}

int Game::GetFireRateUpgrades() const {
    return fire_rate_upgrades;
}

int Game::GetDamageUpgrades() const {
    return damage_upgrades;
}

int Game::GetSpeedUpgrades() const {
    return speed_upgrades;
}

int Game::GetBulletSpeedUpgrades() const {
    return bullet_speed_upgrades;
}

int Game::GetClipSizeUpgrades() const {
    return clip_size_upgrades;
}

int Game::GetEnemyCount() const {
    return enemies.size();
}

int Game::GetPlayerHealth() const {
    return player.GetHealth();
}

int Game::GetPlayerMaxHealth() const {
    return player.GetMaxHealth();
}
