#include "player.hpp"
#include <algorithm>
#include <cmath>

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

void Player::Draw(ftxui::Canvas& canvas) const {
    // Draw Player (ship that can move in all directions)
    std::string player_symbol = shield_active ? "⛊" : "🛦";
    canvas.DrawText(pos.x, pos.y, player_symbol, ftxui::Color::Cyan);
}

void Player::MoveLeft() {
    pos.x = std::max(2, pos.x - MOVE_SPEED);
}

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

void Player::MoveDown() {
    if (screen_height > 0) {
        pos.y = std::min(screen_height - 5, pos.y + MOVE_SPEED);
    } else {
        pos.y += MOVE_SPEED;
    }
}

void Player::SetPosition(int x, int y) {
    pos.x = x;
    pos.y = y;
}

void Player::SetBounds(int width, int height) {
    screen_width = width;
    screen_height = height;
}

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

void Player::ActivateShield() {
    shield_active = true;
    shield_hits_remaining = 2;  // Shield can absorb 2 boss hits
}

void Player::DeactivateShield() {
    shield_active = false;
    shield_hits_remaining = 0;
}

bool Player::HasShield() const {
    return shield_active;
}

void Player::AbsorbHit() {
    if (shield_active && shield_hits_remaining > 0) {
        shield_hits_remaining--;
        if (shield_hits_remaining <= 0) {
            DeactivateShield();
        }
    }
}

Position Player::GetPosition() const {
    return pos;
}

int Player::GetHealth() const {
    return health;
}

int Player::GetMaxHealth() const {
    return MAX_HEALTH;
}