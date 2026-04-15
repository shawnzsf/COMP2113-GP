#include "game.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"
#include <algorithm>

Game::Game() {
    player_pos = {WIDTH / 2, HEIGHT - 3};
    SpawnEnemies();
}

void Game::SpawnEnemies() {
    enemies.clear();
    // Spawn enemies in a grid pattern, with difficulty based on wave
    int enemy_count = ENEMY_SPAWN_COUNT + (wave - 1) * 2;  // More enemies per wave
    int start_x = (WIDTH - (enemy_count * 6)) / 2;  // Center the formation
    
    for (int i = 0; i < enemy_count && i < 20; ++i) {  // Cap at 20 enemies
        Enemy e;
        e.pos.x = start_x + (i * 6);
        e.pos.y = 3 + (i / 5) * 3;  // Multiple rows
        e.alive = true;
        enemies.push_back(e);
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
        shoot_requested = true;
        handled = true;
    }

    return handled;
}

void Game::Update() {
    if (game_over) return;

    frame_count++;

    // Update shoot cooldown
    if (shoot_cooldown > 0) {
        shoot_cooldown--;
    }

    // Apply input state for movement
    if (move_left_timer > 0) {
        if (player_pos.x > 2) {
            player_pos.x -= PLAYER_MOVE_SPEED;
        }
        move_left_timer--;
        move_left = move_left_timer > 0;
    }
    if (move_right_timer > 0) {
        if (player_pos.x < WIDTH - 5) {
            player_pos.x += PLAYER_MOVE_SPEED;
        }
        move_right_timer--;
        move_right = move_right_timer > 0;
    }
    if (move_up_timer > 0) {
        if (player_pos.y > 2) {
            player_pos.y -= PLAYER_MOVE_SPEED;
        }
        move_up_timer--;
        move_up = move_up_timer > 0;
    }
    if (move_down_timer > 0) {
        if (player_pos.y < HEIGHT - 5) {
            player_pos.y += PLAYER_MOVE_SPEED;
        }
        move_down_timer--;
        move_down = move_down_timer > 0;
    }

    // Apply shooting state
    if (shoot_requested && shoot_cooldown <= 0) {
        Bullet b;
        b.pos.x = player_pos.x + 1;  // Center of player ship
        b.pos.y = player_pos.y - 1;
        b.active = true;
        player_bullets.push_back(b);
        shoot_cooldown = SHOOT_COOLDOWN;
        shoot_requested = false;
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
    for (auto& b : player_bullets) {
        if (b.active) {
            b.pos.y -= BULLET_MOVE_SPEED;
            // Mark as inactive if out of bounds
            if (b.pos.y < 1) {
                b.active = false;
            }
        }
    }

    // Remove inactive bullets to save memory
    player_bullets.erase(
        std::remove_if(player_bullets.begin(), player_bullets.end(),
            [](const Bullet& b) { return !b.active; }),
        player_bullets.end()
    );
}

void Game::MoveEnemies() {
    // Simple left-right movement with direction reversal at boundaries
    static int direction = 1;
    
    if (frame_count % ENEMY_MOVE_INTERVAL == 0) {
        bool edge_reached = false;
        
        // Check if any alive enemy reached the edge
        for (const auto& e : enemies) {
            if (e.alive) {
                if ((direction > 0 && e.pos.x >= WIDTH - 6) || 
                    (direction < 0 && e.pos.x <= 4)) {
                    edge_reached = true;
                    break;
                }
            }
        }

        if (edge_reached) {
            // Reverse direction and descend slightly
            direction = -direction;
            for (auto& e : enemies) {
                if (e.alive) {
                    e.pos.y += 2;  // Descend when hitting edge
                }
            }
        } else {
            // Move in current direction
            for (auto& e : enemies) {
                if (e.alive) {
                    e.pos.x += direction * ENEMY_MOVE_SPEED;
                }
            }
        }
    }
}

void Game::CheckCollisions() {
    for (auto& b : player_bullets) {
        if (!b.active) continue;

        for (auto& e : enemies) {
            if (e.alive && 
                std::abs(b.pos.x - e.pos.x) <= COLLISION_RADIUS && 
                std::abs(b.pos.y - e.pos.y) <= COLLISION_RADIUS) {
                
                e.alive = false;
                b.active = false;
                score += POINTS_PER_ENEMY;
                break;  // Bullet only hits one enemy
            }
        }
    }
}

void Game::UpdateWave() {
    // Check if all enemies are defeated
    bool all_defeated = true;
    for (const auto& e : enemies) {
        if (e.alive) {
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

    // Draw Player (ship that can move in all directions)
    canvas.DrawText(player_pos.x, player_pos.y, "▲", ftxui::Color::Cyan);
    // canvas.DrawText(player_pos.x - 1, player_pos.y + 1, "██", ftxui::Color::Cyan);  // Wider base

    // Draw Enemies
    for (const auto& e : enemies) {
        if (e.alive) {
            canvas.DrawText(e.pos.x, e.pos.y, "▼", ftxui::Color::Red);
        }
    }

    // Draw Player Bullets
    for (const auto& b : player_bullets) {
        if (b.active) {
            canvas.DrawText(b.pos.x, b.pos.y, "•", ftxui::Color::Yellow);
        }
    }

    // Draw HUD (Score and Wave)
    std::string score_text = "Score: " + std::to_string(score);
    std::string wave_text = "Wave: " + std::to_string(wave);
    std::string enemies_text = "Enemies: " + std::to_string(
        std::count_if(enemies.begin(), enemies.end(), 
            [](const Enemy& e) { return e.alive; })
    );
    
    canvas.DrawText(2, 1, score_text, ftxui::Color::Green);
    canvas.DrawText(WIDTH / 2 - 8, 1, wave_text, ftxui::Color::Cyan);
    canvas.DrawText(WIDTH - 20, 1, enemies_text, ftxui::Color::Magenta);

    // Draw game over message
    if (game_over) {
        canvas.DrawText(WIDTH / 2 - 5, HEIGHT / 2, "GAME OVER", ftxui::Color::Red);
        canvas.DrawText(WIDTH / 2 - 8, HEIGHT / 2 + 2, "Final Score: " + std::to_string(score), ftxui::Color::White);
        canvas.DrawText(WIDTH / 2 - 7, HEIGHT / 2 + 4, "Press Q to quit", ftxui::Color::Yellow);
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
