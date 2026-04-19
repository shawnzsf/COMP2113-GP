/**
 * @file game.cpp
 * @brief Core game engine implementation
 *
 * This file contains the main Game class implementation.
 * Handles: game initialization, update loop, enemy spawning,
 * collision detection, wave progression, and random events.
 */

#include "game.hpp"
#include "types.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>

// Game constructor - initializes a new game session
// Inputs: None | Outputs: None
Game::Game() {
    srand(time(nullptr));  // Initialize random seed
    player.SetBounds(WIDTH, HEIGHT);
    player.SetPosition(WIDTH / 2, HEIGHT / 2);

    // Initialize weapon
    current_weapon = CreateWeapon(WeaponType::BASIC);
    weapon_type = WeaponType::BASIC;

    // Initialize empty event
    current_event = {"", "", EventType::PLAYER_BUFF, 0};
    event_timer = 0;
    current_wave_has_event = false;

    SpawnEnemies();
}

// Spawn enemies for the current wave using toughness system
// Inputs: None (uses current wave number) | Outputs: None (populates enemies vector)
void Game::SpawnEnemies() {
    enemies.clear();

    // Toughness system: each wave has a fixed toughness value
    int base_toughness = 2;
    int wave_toughness = base_toughness + (wave - 1) * 2;  // Toughness increases by 2 each wave

    // Enemy toughness costs
    const int REGULAR_COST = 1;
    const int ELITE_COST = 3;
    const int CIRCLE_COST = 4;
    const int BOSS_COST = 6;
    const int MEGABOSS_COST = 10;

    // Determine available enemy types based on wave
    std::vector<std::pair<EnemyType, int>> available_enemies;
    available_enemies.push_back({EnemyType::REGULAR, REGULAR_COST});

    // Elite available from wave 1
    available_enemies.push_back({EnemyType::ELITE, ELITE_COST});

    // Circle shooter from wave 3
    if (wave >= 3) {
        available_enemies.push_back({EnemyType::CIRCLE_SHOOTER, CIRCLE_COST});
    }

    // Boss from wave 5
    if (wave >= 5) {
        available_enemies.push_back({EnemyType::BOSS, BOSS_COST});
    }

    // Megaboss from wave 10
    if (wave >= 10) {
        available_enemies.push_back({EnemyType::MEGABOSS, MEGABOSS_COST});
    }

    // Dropship from wave 12
    const int DROPSHIP_COST = 15;
    if (wave >= 12) {
        available_enemies.push_back({EnemyType::DROPSHIP, DROPSHIP_COST});
    }

    // Calculate difficulty multiplier (HP scales faster - every 3 waves +1 HP)
    int difficulty_multiplier = 1 + (wave / 3);
    // Apply difficulty level HP multiplier
    difficulty_multiplier = std::max(static_cast<int>(difficulty_multiplier * GetEnemyHealthMultiplier()), 1);

    // Cash reward multiplier - decreases over time, then apply difficulty level
    float wave_cash_mult = std::max(0.3f, 1.0f - static_cast<float>(wave) * 0.03f);
    float diff_cash_mult = GetCashRewardMultiplier();
    cash_multiplier_cache = wave_cash_mult * diff_cash_mult;

    // Fill toughness with available enemies
    int remaining_toughness = static_cast<int>(wave_toughness * GetToughnessMultiplier());
    std::vector<Position> positions;

    // Generate potential positions with randomized horizontal offset
    int max_enemies = 30;
    int start_x = (WIDTH - (max_enemies * 4)) / 2;
    // Random offset between -25 and 25 for varied starting positions
    int random_offset = (rand() % 50) - 25;
    for (int i = 0; i < max_enemies; i++) {
        positions.push_back({start_x + (i * 4) + random_offset, 3 + (i / 5) * 3});
    }

    int pos_index = 0;
    while (remaining_toughness > 0 && pos_index < (int)positions.size()) {
        // Filter available enemies that fit in remaining toughness
        std::vector<std::pair<EnemyType, int>> viable;
        for (const auto& e : available_enemies) {
            if (e.second <= remaining_toughness) {
                viable.push_back(e);
            }
        }

        if (viable.empty()) break;  // No more enemies can fit

        // Pick random enemy from viable options (favor cheaper ones)
        int pick = rand() % viable.size();

        // Higher chance for cheaper enemies
        if (rand() % 100 < 60 && viable.size() > 1) {
            // Try to pick a cheaper one
            pick = rand() % (viable.size() / 2 + 1);
        }

        EnemyType etype = viable[pick].first;
        int cost = viable[pick].second;

        Enemy enemy(etype, positions[pos_index]);

        // Scale health by difficulty
        enemy.max_health *= difficulty_multiplier;
        enemy.health = enemy.max_health;

        enemies.push_back(enemy);
        remaining_toughness -= cost;
        pos_index++;
    }

    // Ensure at least some enemies spawn
    if (enemies.empty()) {
        Enemy regular(EnemyType::REGULAR, {WIDTH / 2, 3});
        regular.max_health = difficulty_multiplier;
        regular.health = regular.max_health;
        enemies.push_back(regular);
    }
}

// Handle user keyboard input for movement, shooting, weapon switching
// Inputs: const ftxui::Event& event | Outputs: bool (true if handled)
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

    // Handle bullet type switching
    if (event == ftxui::Event::Character('1')) {
        current_bullet_type = BulletType::NORMAL;
        handled = true;
    }
    if (event == ftxui::Event::Character('2') && CanUseBulletType(BulletType::EXPLOSIVE)) {
        current_bullet_type = BulletType::EXPLOSIVE;
        handled = true;
    }
    if (event == ftxui::Event::Character('3') && CanUseBulletType(BulletType::PIERCING)) {
        current_bullet_type = BulletType::PIERCING;
        handled = true;
    }

    // Handle weapon switching
    if (event == ftxui::Event::Character('z') || event == ftxui::Event::Character('Z')) {
        weapon_type = WeaponType::BASIC;
        current_weapon = CreateWeapon(weapon_type);
        // If current bullet type is not allowed, switch to normal
        if (!CanUseBulletType(current_bullet_type)) {
            current_bullet_type = BulletType::NORMAL;
        }
        handled = true;
    }
    if (event == ftxui::Event::Character('x') || event == ftxui::Event::Character('X')) {
        if (has_dual_weapon) {
            weapon_type = WeaponType::DUAL;
            current_weapon = CreateWeapon(weapon_type);
            if (!CanUseBulletType(current_bullet_type)) {
                current_bullet_type = BulletType::NORMAL;
            }
        }
        handled = true;
    }
    if (event == ftxui::Event::Character('c') || event == ftxui::Event::Character('C')) {
        if (has_tri_weapon) {
            weapon_type = WeaponType::TRI;
            current_weapon = CreateWeapon(weapon_type);
            if (!CanUseBulletType(current_bullet_type)) {
                current_bullet_type = BulletType::NORMAL;
            }
        }
        handled = true;
    }

    return handled;
}

// Main game update loop - called every frame (~60 FPS)
// Inputs: None | Outputs: None
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
        int current_cooldown = SHOOT_COOLDOWN;
        // Rapid fire from items or events
        if (has_rapid_fire || current_event.name == "Super Charge!") {
            current_cooldown = 3;
        }
        shoot_cooldown = current_cooldown;
    }

    // Apply speed boost from items or events
    if (speed_boost_timer > 0 || current_event.name == "Lightning Speed!") {
        move_left_timer = std::min(move_left_timer, 4);
        move_right_timer = std::min(move_right_timer, 4);
        move_up_timer = std::min(move_up_timer, 4);
        move_down_timer = std::min(move_down_timer, 4);
    }

    // Update game logic
    MoveBullets();
    MoveEnemies();
    CheckCollisions();
    UpdateWave();

    // Update item timers (60 frames = 1 second)
    if (speed_boost_timer > 0) speed_boost_timer--;
    if (damage_boost_timer > 0) damage_boost_timer--;
    if (shield_pack_timer > 0) {
        shield_pack_timer--;
        if (shield_pack_timer == 0) {
            player.DeactivateShield();
        } else if (!player.HasShield()) {
            player.ActivateShield();
        }
    }
    if (time_slow_timer > 0) time_slow_timer--;
    if (freeze_timer > 0) freeze_timer--;

    // Enemy descent (independent of movement)
    if (frame_count % ENEMY_DESCENT_INTERVAL == 0) {
        for (auto& e : enemies) {
            if (e.alive && e.type != EnemyType::DROPSHIP) e.pos.y++;
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

// Move all player bullets upward, remove off-screen bullets
// Inputs: None | Outputs: None
void Game::MoveBullets() {
    // Limit total bullets to prevent performance issues
    const int MAX_BULLETS = 200;

    for (auto& b : player_bullets) {
        if (!b.active) continue;

        b.pos.x += b.dx;
        b.pos.y += b.dy;

        // Remove if out of bounds
        if (b.pos.y < 1 || b.pos.y >= HEIGHT - 1 || b.pos.x < 1 || b.pos.x >= WIDTH - 1) {
            b.active = false;
        }
    }

    // Remove inactive bullets to save memory
    player_bullets.erase(
        std::remove_if(player_bullets.begin(), player_bullets.end(),
            [](const Bullet& b) { return !b.active; }),
        player_bullets.end()
    );

    // If too many bullets, remove oldest ones (from the front of vector)
    while (player_bullets.size() > MAX_BULLETS && !player_bullets.empty()) {
        player_bullets.erase(player_bullets.begin());
    }
}

void Game::FireWeapon() {
    if (!current_weapon) return;

    Position player_pos = player.GetPosition();
    BulletType bullet_type = current_bullet_type;

    // Apply damage boosts (from items and events)
    int damage_multiplier = 1;
    if (damage_boost_timer > 0) {
        damage_multiplier *= 2;
    }
    if (current_event.name == "Power Surge!") {
        damage_multiplier *= 2;
    }
    if (current_event.name == "Jammed!") {
        damage_multiplier = std::max(1, damage_multiplier - 1);
    }

    // Calculate damage based on bullet type
    int bullet_damage = damage_multiplier * (
        (bullet_type == BulletType::NORMAL) ? basic_bullet_damage :
        (bullet_type == BulletType::EXPLOSIVE) ? explosive_bullet_damage :
        piercing_bullet_damage);

    // Adjust bullet speed upgrades
    int base_speed = 1 + bullet_speed_upgrades;
    if (bullet_type == BulletType::PIERCING) {
        base_speed = piercing_bullet_speed + bullet_speed_upgrades;
    }

    // Check if weapon can use this bullet type
    if (!current_weapon->CanUseBulletType(bullet_type)) {
        bullet_type = BulletType::NORMAL;
        bullet_damage = damage_multiplier * basic_bullet_damage;
    }

    // Fire weapon and get bullets
    auto bullets = current_weapon->Fire(player_pos, bullet_type, bullet_damage);

    // Apply speed adjustments
    for (auto& b : bullets) {
        if (bullet_type == BulletType::PIERCING) {
            b.dy = -(base_speed / 2);
            b.penetration = piercing_bullet_penetration;
        } else {
            b.dy = -base_speed;
        }
        player_bullets.push_back(b);
    }
}

// Move all enemies, update state, handle shooting patterns
// Inputs: None | Outputs: None
void Game::MoveEnemies() {
    // Handle formation movement for regular enemies
    static int direction = 1;

    // Determine effective move interval based on abilities
    int effective_move_interval = ENEMY_MOVE_INTERVAL;
    if (time_slow_timer > 0) {
        effective_move_interval *= 2;  // Slow down by 50%
    }

    // Check if enemies should be frozen
    bool enemies_frozen = (freeze_timer > 0);

    if (!enemies_frozen && frame_count % effective_move_interval == 0) {
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

    // Add spawned enemies
    for (auto& e : enemies) {
        enemies.insert(enemies.end(), e.spawned_enemies.begin(), e.spawned_enemies.end());
        e.spawned_enemies.clear();
    }
}

// Check bullet-enemy and enemy bullet-player collisions
// Inputs: None | Outputs: None
void Game::CheckCollisions() {
    // Calculate difficulty multiplier for rewards
    int difficulty = GetDifficultyMultiplier();

    for (auto& b : player_bullets) {
        if (!b.active) continue;

        for (auto& e : enemies) {
            float collision_radius = 1.0f;
            if (e.type == EnemyType::BOSS) collision_radius = 2.0f;
            else if (e.type == EnemyType::MEGABOSS) collision_radius = 3.0f;
            else if (e.type == EnemyType::DROPSHIP) collision_radius = 4.0f;

            if (b.type == BulletType::EXPLOSIVE) collision_radius *= 2.0f;
            else if (b.type == BulletType::NORMAL) collision_radius *= 1.5f;

            if (e.IsAlive() &&
                std::abs(static_cast<float>(b.pos.x - e.pos.x)) <= collision_radius &&
                std::abs(static_cast<float>(b.pos.y - e.pos.y)) <= collision_radius) {

                // Deal damage to enemy based on bullet type
                if (b.type == BulletType::EXPLOSIVE) {
                    // Explosive: spawn bullets in a circle (like boss circle attack)
                    const int bullet_count = 16;  // Number of spread bullets
                    for (int i = 0; i < bullet_count; ++i) {
                        double angle = 2.0 * 3.14159265358979323846 * i / bullet_count;
                        int dx = static_cast<int>(std::round(std::cos(angle)));
                        int dy = static_cast<int>(std::round(std::sin(angle)));
                        if (dx == 0 && dy == 0) {
                            dx = 0; dy = -1;
                        }

                        Bullet fragment;
                        fragment.type = BulletType::NORMAL;
                        fragment.active = true;
                        fragment.pos = b.pos;
                        fragment.dx = dx;
                        fragment.dy = dy;
                        fragment.damage = explosive_bullet_damage;
                        player_bullets.push_back(fragment);
                    }
                    b.active = false;  // Explosive consumed on impact
                } else if (b.type == BulletType::PIERCING) {
                    // Piercing: damage but don't destroy bullet, decrement penetration
                    e.TakeDamage(piercing_bullet_damage);
                    b.penetration--;
                    if (b.penetration <= 0) {
                        b.active = false;
                    }
                    // Award points if enemy destroyed
                    if (!e.IsAlive()) {
                        switch (e.type) {
                            case EnemyType::REGULAR: score += POINTS_REGULAR_ENEMY * difficulty; cash += static_cast<int>(CASH_REGULAR_ENEMY * cash_multiplier_cache); break;
                            case EnemyType::ELITE: score += POINTS_ELITE_ENEMY * difficulty; cash += static_cast<int>(CASH_ELITE_ENEMY * cash_multiplier_cache); break;
                            case EnemyType::BOSS: score += POINTS_BOSS_ENEMY * difficulty; cash += static_cast<int>(CASH_BOSS_ENEMY * cash_multiplier_cache); break;
                            case EnemyType::CIRCLE_SHOOTER: score += 60 * difficulty; cash += static_cast<int>(CASH_CIRCLE_SHOOTER * cash_multiplier_cache); break;
                            case EnemyType::MEGABOSS: score += 200 * difficulty; cash += static_cast<int>(CASH_MEGABOSS * cash_multiplier_cache); break;
                            case EnemyType::DROPSHIP: score += POINTS_DROPSHIP_ENEMY * difficulty; cash += static_cast<int>(CASH_DROPSHIP_ENEMY * cash_multiplier_cache); break;
                        }
                    }
                } else {
                    // Normal bullet
                    e.TakeDamage(b.damage);
                    b.active = false;
                    if (!e.IsAlive()) {
                        switch (e.type) {
                            case EnemyType::REGULAR: score += POINTS_REGULAR_ENEMY * difficulty; cash += static_cast<int>(CASH_REGULAR_ENEMY * cash_multiplier_cache); break;
                            case EnemyType::ELITE: score += POINTS_ELITE_ENEMY * difficulty; cash += static_cast<int>(CASH_ELITE_ENEMY * cash_multiplier_cache); break;
                            case EnemyType::BOSS: score += POINTS_BOSS_ENEMY * difficulty; cash += static_cast<int>(CASH_BOSS_ENEMY * cash_multiplier_cache); break;
                            case EnemyType::CIRCLE_SHOOTER: score += 60 * difficulty; cash += static_cast<int>(CASH_CIRCLE_SHOOTER * cash_multiplier_cache); break;
                            case EnemyType::MEGABOSS: score += 200 * difficulty; cash += static_cast<int>(CASH_MEGABOSS * cash_multiplier_cache); break;
                            case EnemyType::DROPSHIP: score += POINTS_DROPSHIP_ENEMY * difficulty; cash += static_cast<int>(CASH_DROPSHIP_ENEMY * cash_multiplier_cache); break;
                        }
                    }
                }

                break;  // Bullet moves to next target (if piercing still active)
            }
        }
    }

    // Check collisions between enemy bullets and player
    for (auto& e : enemies) {
        if (e.type == EnemyType::BOSS || e.type == EnemyType::CIRCLE_SHOOTER || e.type == EnemyType::MEGABOSS || e.type == EnemyType::DROPSHIP) {
            for (auto& enemy_bullet : e.bullets) {
                if (!enemy_bullet.active) continue;

                Position player_pos = player.GetPosition();
                float bullet_radius = (enemy_bullet.type == BulletType::NORMAL) ? 1.5f : 1.0f;
                if (std::abs(static_cast<float>(enemy_bullet.pos.x - player_pos.x)) <= bullet_radius &&
                    std::abs(static_cast<float>(enemy_bullet.pos.y - player_pos.y)) <= bullet_radius) {

                    // Apply enemy damage multiplier
                    int damage_to_player = std::max(1, static_cast<int>(1 * GetEnemyDamageMultiplier()));
                    player.TakeDamage(damage_to_player);
                    enemy_bullet.active = false;

                    if (!player.IsAlive()) {
                        game_over = true;
                        return;
                    }
                    break;
                }
            }
        }
    }
}

// Check if wave complete, increment wave, spawn new enemies
// Inputs: None | Outputs: None
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

        // Generate random event every 2 waves
        if (wave >= 2 && wave % 2 == 0) {
            GenerateRandomEvent();
        } else if (wave < 2) {
            current_event = {"Clear Skies", "No event this wave", EventType::PLAYER_BUFF, 0};
            current_wave_has_event = false;
        }
    }
}

// Draw player, bullets, enemies to canvas
// Inputs: ftxui::Canvas& canvas | Outputs: None
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
        b.Draw(canvas);
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

    // Game over message is now shown in the dedicated game over screen (main.cpp)
}

// Check if game is over (player dead)
// Inputs: None | Outputs: bool (true if game over)
bool Game::IsGameOver() const { return game_over; }

// Get current score
// Inputs: None | Outputs: int (current score)
int Game::GetScore() const { return score; }

// Get current wave number
// Inputs: None | Outputs: int (current wave)
int Game::GetWave() const { return wave; }

// Check if player can afford an item
// Inputs: int amount - cost to check | Outputs: bool (true if can afford)
bool Game::CanAfford(int amount) const { return cash >= amount; }

bool Game::BuyWeapon(WeaponType type, int cost) {
    if (type == weapon_type || !CanAfford(cost)) {
        return false;
    }
    cash -= cost;
    if (type == WeaponType::DUAL) has_dual_weapon = true;
    if (type == WeaponType::TRI) has_tri_weapon = true;
    SetWeaponType(type);
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

void Game::SetWeaponType(WeaponType type) {
    weapon_type = type;
    current_weapon = CreateWeapon(type);
    // If current bullet type is not allowed with new weapon, switch to normal
    if (!CanUseBulletType(current_bullet_type)) {
        current_bullet_type = BulletType::NORMAL;
    }
}

void Game::SetDifficulty(DifficultyLevel level) {
    difficulty = level;
}

DifficultyLevel Game::GetDifficulty() const {
    return difficulty;
}

// Get difficulty modifiers based on selected difficulty level
float Game::GetCashRewardMultiplier() const {
    switch (difficulty) {
        case DifficultyLevel::Easy:    return 2.0f;   // 100% more cash
        case DifficultyLevel::Medium:  return 1.5f;   // 150% more cash
        case DifficultyLevel::Hard:   return 1.0f;   // Normal
    }
    return 1.0f;
}

float Game::GetEnemyDamageMultiplier() const {
    switch (difficulty) {
        case DifficultyLevel::Easy:    return 0.7f;   // 30% less damage
        case DifficultyLevel::Medium:  return 0.9f;   // Slightly easier
        case DifficultyLevel::Hard:   return 1.5f;   // 50% more damage
    }
    return 1.0f;
}

float Game::GetEnemyHealthMultiplier() const {
    switch (difficulty) {
        case DifficultyLevel::Easy:    return 0.7f;   // 30% less HP
        case DifficultyLevel::Medium:  return 0.9f;   // Slightly easier
        case DifficultyLevel::Hard:   return 1.5f;   // 50% more HP
    }
    return 1.0f;
}

float Game::GetToughnessMultiplier() const {
    switch (difficulty) {
        case DifficultyLevel::Easy:    return 0.5f;   // Much fewer enemies
        case DifficultyLevel::Medium:  return 0.7f;   // Fewer enemies
        case DifficultyLevel::Hard:   return 1.4f;   // 40% more toughness (more enemies)
    }
    return 1.0f;
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

// Bullet type accessors
BulletType Game::GetBulletType() const {
    return current_bullet_type;
}

void Game::SetBulletType(BulletType type) {
    if (CanUseBulletType(type)) {
        current_bullet_type = type;
    }
}

bool Game::CanUseBulletType(BulletType type) const {
    if (type == BulletType::NORMAL) return true;
    if (type == BulletType::EXPLOSIVE) {
        if (!owned_explosive) return false;
        // Cannot use explosive with multi-shot weapons
        if (weapon_type == WeaponType::DUAL || weapon_type == WeaponType::TRI) return false;
        return true;
    }
    if (type == BulletType::PIERCING) return owned_piercing;
    return false;
}

bool Game::HasExplosive() const {
    return owned_explosive;
}

bool Game::HasPiercing() const {
    return owned_piercing;
}

bool Game::BuyExplosiveBullet(int cost) {
    if (owned_explosive || !CanAfford(cost)) return false;
    cash -= cost;
    owned_explosive = true;
    return true;
}

bool Game::BuyPiercingBullet(int cost) {
    if (owned_piercing || !CanAfford(cost)) return false;
    cash -= cost;
    owned_piercing = true;
    return true;
}

bool Game::UpgradeBasicBulletDamage(int cost) {
    if (!CanAfford(cost) || basic_bullet_damage >= 6) return false;  // Limit to +5 damage (base 1 + 5 = 6)
    cash -= cost;
    basic_bullet_damage++;
    return true;
}

bool Game::UpgradeBasicBulletSpeed(int cost) {
    if (!CanAfford(cost) || bullet_speed_upgrades >= 4) return false;  // Limit to +4 speed upgrades
    cash -= cost;
    bullet_speed_upgrades++;
    return true;
}

bool Game::UpgradeExplosiveDamage(int cost) {
    if (!CanAfford(cost) || explosive_bullet_damage >= 4) return false;  // Limit to +3 damage (base 1 + 3 = 4)
    cash -= cost;
    explosive_bullet_damage++;
    return true;
}

bool Game::UpgradeExplosiveRadius(int cost) {
    if (!CanAfford(cost) || explosive_bullet_radius >= 4) return false;  // Limit to +3 radius (base 1 + 3 = 4)
    cash -= cost;
    explosive_bullet_radius++;
    return true;
}

bool Game::UpgradePiercingDamage(int cost) {
    if (!CanAfford(cost) || piercing_bullet_damage >= 7) return false;  // Limit to +4 damage (base 2 + 4 = 6)
    cash -= cost;
    piercing_bullet_damage++;
    return true;
}

bool Game::UpgradePiercingPenetration(int cost) {
    if (!CanAfford(cost) || piercing_bullet_penetration >= 5) return false;  // Limit to +2 penetration (base 2 + 2 = 4)
    cash -= cost;
    piercing_bullet_penetration++;
    return true;
}

bool Game::BuySpeedBoost(int cost) {
    if (!CanAfford(cost)) return false;
    cash -= cost;
    speed_boost_timer = 600;  // 10 seconds at 60fps
    return true;
}

bool Game::BuyHealthPack(int cost) {
    if (!CanAfford(cost)) return false;
    cash -= cost;
    player.TakeDamage(-1);  // Negative damage = healfr
    return true;
}

bool Game::BuyShieldPack(int cost) {
    if (!CanAfford(cost)) return false;
    cash -= cost;
    shield_pack_timer = 1800;  // 30 seconds at 60fps
    player.ActivateShield();
    return true;
}

bool Game::BuyDamageBoost(int cost) {
    if (!CanAfford(cost)) return false;
    cash -= cost;
    damage_boost_timer = 480;  // 8 seconds at 60fps
    return true;
}

bool Game::BuyRapidFire(int cost) {
    if (!CanAfford(cost)) return false;
    cash -= cost;
    has_rapid_fire = true;
    rapid_fire_level++;
    return true;
}

bool Game::BuyTimeSlow(int cost) {
    if (!CanAfford(cost)) return false;
    cash -= cost;
    has_time_slow = true;
    return true;
}

bool Game::BuyFreeze(int cost) {
    if (!CanAfford(cost)) return false;
    cash -= cost;
    has_freeze = true;
    return true;
}

// Item/ability accessors
int Game::GetSpeedBoostTimer() const {
    return speed_boost_timer;
}

int Game::GetDamageBoostTimer() const {
    return damage_boost_timer;
}

bool Game::HasFreezeAbility() const {
    return has_freeze;
}

int Game::GetTimeSlowTimer() const {
    return time_slow_timer;
}

int Game::GetFreezeTimer() const {
    return freeze_timer;
}

void Game::ActivateTimeSlow() {
    if (has_time_slow) {
        time_slow_timer = 300;  // 5 seconds at 60fps
    }
}

void Game::ActivateFreeze() {
    if (has_freeze) {
        freeze_timer = 300;  // 5 seconds at 60fps
    }
}

// Random event system
void Game::GenerateRandomEvent() {
    // Library of 19 predefined events
    const std::vector<RandomEvent> event_library = {
        // Player Buffs (good for player)
        {"Super Charge!", "Fire rate doubled for this wave", EventType::PLAYER_BUFF, 0},
        {"Power Surge!", "+50% bullet damage for this wave", EventType::PLAYER_BUFF, 0},
        {"Lightning Speed!", "+50% movement speed for this wave", EventType::PLAYER_BUFF, 0},
        {"Quick Cash!", "+50% cash from enemies this wave", EventType::PLAYER_BUFF, 0},
        {"Triple Threat!", "Start with Tri Shot unlocked", EventType::PLAYER_BUFF, 0},
        {"Health Regen!", "Restore 1 HP instantly", EventType::PLAYER_BUFF, 0},
        {"Shield Boost!", "Activate shield for 15 seconds", EventType::PLAYER_BUFF, 0},

        // Enemy Debuffs (good for player)
        {"Time Dilation!", "Enemies move 50% slower this wave", EventType::ENEMY_DEBUFF, 0},
        {"Fatigue!", "Enemy health reduced by 25%", EventType::ENEMY_DEBUFF, 0},
        {"Scattered!", "Enemy bullet speed reduced by 30%", EventType::ENEMY_DEBUFF, 0},
        {"Blindness!", "Boss accuracy reduced this wave", EventType::ENEMY_DEBUFF, 0},

        // Enemy Buffs (bad for player)
        {"Frenzy!", "Enemies move 30% faster this wave", EventType::ENEMY_BUFF, 0},
        {"Armored!", "Enemy health increased by 25%", EventType::ENEMY_BUFF, 0},
        {"Ruthless!", "Enemies deal +50% damage", EventType::ENEMY_BUFF, 0},
        {"Swarm!", "20% more enemies spawn this wave", EventType::ENEMY_BUFF, 0},
        {"Aggression!", "Enemies shoot 25% faster", EventType::ENEMY_BUFF, 0},

        // Player Debuffs (bad for player)
        {"Jammed!", "Fire rate reduced by 30%", EventType::PLAYER_DEBUFF, 0},
        {"Leaky Shield!", "Shield effectiveness -25%", EventType::PLAYER_DEBUFF, 0},
        {"Drought!", "-25% cash from enemies", EventType::PLAYER_DEBUFF, 0}
    };

    // Pick random event
    int event_idx = rand() % event_library.size();
    current_event = event_library[event_idx];
    event_timer = 0;  // Event applies for the whole wave
    current_wave_has_event = true;

    // Apply immediate effects
    ApplyEventEffect();
}

void Game::ApplyEventEffect() {
    switch (current_event.type) {
        case EventType::PLAYER_BUFF:
            if (current_event.name == "Health Regen!") {
                player.TakeDamage(-1);
            } else if (current_event.name == "Shield Boost!") {
                player.ActivateShield();
            } else if (current_event.name == "Triple Threat!") {
                weapon_type = WeaponType::TRI;
            } else if (current_event.name == "Quick Cash!") {
                // Handled in CheckCollisions
            } else if (current_event.name == "Super Charge!") {
                has_rapid_fire = true;
            } else if (current_event.name == "Power Surge!") {
                damage_boost_timer = -1;  // Infinite for this wave (handled in FireWeapon)
            } else if (current_event.name == "Lightning Speed!") {
                speed_boost_timer = -1;  // Infinite for this wave
            }
            break;

        case EventType::ENEMY_DEBUFF:
            // These are handled in respective update methods
            break;

        case EventType::ENEMY_BUFF:
            // These are handled in SpawnEnemies and enemy update
            break;

        case EventType::PLAYER_DEBUFF:
            // These are handled in respective update methods
            break;
    }
}

const RandomEvent& Game::GetCurrentEvent() const {
    return current_event;
}

bool Game::HasActiveEvent() const {
    return current_wave_has_event;
}
