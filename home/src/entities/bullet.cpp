/**
 * @file bullet.cpp
 * @brief Bullet entity implementation
 *
 * Handles bullet types: Basic, Explosive, Piercing
 */

#include "bullet.hpp"
#include <cmath>
#include <algorithm>

// Constructor - initialize bullet
// Inputs: Position, BulletType, int damage | Outputs: None
Bullet::Bullet(Position start_pos, BulletType bullet_type, int bullet_damage)
    : pos(start_pos), type(bullet_type), damage(bullet_damage) {
    active = true;
    dx = 0;
    dy = -1;
}

void Bullet::Update() {
    if (!active) return;
    pos.x += dx;
    pos.y += dy;
    lifetime++;
}

// Update bullet position
// Inputs: None | Outputs: None

// Get bullet display symbol
// Inputs: None | Outputs: std::string
std::string Bullet::GetSymbol() const {
    if (type == BulletType::EXPLOSIVE) return "@";
    if (type == BulletType::PIERCING) return "╹";
    return "•";
}

ftxui::Color Bullet::GetColor() const {
    if (type == BulletType::EXPLOSIVE) return ftxui::Color::Yellow;
    if (type == BulletType::PIERCING) return ftxui::Color::Cyan;
    return ftxui::Color::Yellow;
}

void Bullet::Draw(ftxui::Canvas& canvas) const {
    if (!active) return;
    canvas.DrawText(pos.x, pos.y, GetSymbol(), GetColor());
}

// Factory functions
Bullet CreateBasicBullet(Position pos, BulletType type, int damage) {
    Bullet b;
    b.pos = pos;
    b.type = type;
    b.damage = damage;
    b.dx = 0;
    b.dy = -1;
    b.active = true;
    return b;
}

Bullet CreateExplosiveBullet(Position pos, int damage) {
    Bullet b;
    b.pos = pos;
    b.type = BulletType::EXPLOSIVE;
    b.damage = damage;
    b.dx = 0;
    b.dy = -1;
    b.active = true;
    return b;
}

Bullet CreatePiercingBullet(Position pos, int damage, int penetration) {
    Bullet b;
    b.pos = pos;
    b.type = BulletType::PIERCING;
    b.damage = damage;
    b.penetration = penetration;
    b.dx = 0;
    b.dy = -2;  // Faster
    b.active = true;
    return b;
}