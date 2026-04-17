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
    std::vector<std::string> sprite = {
        " /\\ ",
        "/++\\",
        " || "
    };

    for (size_t dy = 0; dy < sprite.size(); ++dy) {
        canvas.DrawText(pos.x, pos.y + static_cast<int>(dy), sprite[dy], ftxui::Color::Cyan);
    }

    if (shield_active) {
        // Draw circular shield border around player
        int radius = 4;  // Adjusted for larger sprite
        for (int angle = 0; angle < 360; angle += 10) {  // Draw points every 10 degrees for smoother circle
            double rad = angle * 3.14159 / 180.0;
            int x = pos.x + 2 + static_cast<int>(radius * cos(rad));  // Center on sprite
            int y = pos.y + 1 + static_cast<int>(radius * sin(rad));
            if ((screen_width == 0 || (x >= 0 && x < screen_width)) &&
                (screen_height == 0 || (y >= 0 && y < screen_height))) {
                canvas.DrawText(x, y, "○", ftxui::Color::Cyan);  // Use a circle symbol
            }
        }
    }
}

void Player::MoveLeft() {
    pos.x = std::max(2, pos.x - MOVE_SPEED);
}

void Player::MoveRight() {
    if (screen_width > 0) {
        pos.x = std::min(screen_width - WIDTH - 1, pos.x + MOVE_SPEED);
    } else {
        pos.x += MOVE_SPEED;
    }
}

void Player::MoveUp() {
    pos.y = std::max(2, pos.y - MOVE_SPEED);
}

void Player::MoveDown() {
    if (screen_height > 0) {
        pos.y = std::min(screen_height - HEIGHT - 1, pos.y + MOVE_SPEED);
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
    shield_hits_remaining = 1;  // Shield can absorb 1 boss hit
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