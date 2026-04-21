/**
 * @file player.cpp
 * @brief Player entity implementation
 *
 * Handles player state: position, health, movement, shield.
 */

#include "player.hpp"
#include <algorithm>
#include <cmath>

// Constructor - initialize player at given position
// Inputs: int start_x, int start_y | Outputs: None
Player::Player(int start_x, int start_y) {
    pos = Position{start_x, start_y};
    health = MAX_HEALTH;
    shield_active = false;
    shield_hits_remaining = 0;
    screen_width = 0;
    screen_height = 0;
}

void Player::Update() {
    // Player update logic (if needed for animations, etc.)
}

// Draw player to canvas
// Inputs: ftxui::Canvas& canvas | Outputs: None
void Player::Draw(ftxui::Canvas& canvas) const {
    // Draw Player (ship that can move in all directions)
    std::string player_symbol = shield_active ? "⛊" : "▲";
    canvas.DrawText(pos.x, pos.y, player_symbol, ftxui::Color::Cyan);
}

// Move player left by MOVE_SPEED
// Inputs: None | Outputs: None
void Player::MoveLeft() {
    pos.x = std::max(2, pos.x - MOVE_SPEED);
}

// Move player right by MOVE_SPEED
// Inputs: None | Outputs: None
void Player::MoveRight() {
    if (screen_width > 0) {
        pos.x = std::min(screen_width - 5, pos.x + MOVE_SPEED);
    } else {
        pos.x += MOVE_SPEED;
    }
}

void Player::MoveUp() {
    pos.y = std::max(2, pos.y - MOVE_SPEED);
}

// Move player down
// Inputs: None | Outputs: None
void Player::MoveDown() {
    if (screen_height > 0) {
        pos.y = std::min(screen_height - 5, pos.y + MOVE_SPEED);
    } else {
        pos.y += MOVE_SPEED;
    }
}

// Set player position
// Inputs: int x, int y | Outputs: None
void Player::SetPosition(int x, int y) {
    pos.x = x;
    pos.y = y;
}

// Set screen bounds
// Inputs: int width, int height | Outputs: None
void Player::SetBounds(int width, int height) {
    screen_width = width;
    screen_height = height;
}

// Take damage (shield absorbs first)
// Inputs: int damage | Outputs: None
void Player::TakeDamage(int damage) {
    if (shield_active) {
        AbsorbHit();
    } else {
        health -= damage;
        if (health < 0) health = 0;
    }
}

bool Player::IsAlive() const {
    return health > 0;
}

// Activate shield (absorbs 2 hits)
// Inputs: None | Outputs: None
void Player::ActivateShield() {
    shield_active = true;
    shield_hits_remaining = 2;  // Shield can absorb 2 boss hits
}

void Player::DeactivateShield() {
    shield_active = false;
    shield_hits_remaining = 0;
}

// Check if shield is active
// Inputs: None | Outputs: bool
bool Player::HasShield() const {
    return shield_active;
}

// Handle shield hit absorption
// Inputs: None | Outputs: None
void Player::AbsorbHit() {
    if (shield_active && shield_hits_remaining > 0) {
        shield_hits_remaining--;
        if (shield_hits_remaining <= 0) {
            DeactivateShield();
        }
    }
}

// Get player position
// Inputs: None | Outputs: Position
Position Player::GetPosition() const {
    return pos;
}

// Get current health
// Inputs: None | Outputs: int
int Player::GetHealth() const {
    return health;
}

// Get max health
// Inputs: None | Outputs: int
int Player::GetMaxHealth() const {
    return MAX_HEALTH;
}