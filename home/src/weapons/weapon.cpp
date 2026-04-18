/**
 * @file weapon.cpp
 * @brief Weapon implementation (Strategy pattern)
 *
 * Handles different weapon firing patterns:
 * - BasicWeapon: single shot
 * - DualWeapon: two bullets left/right
 * - TriWeapon: three bullets spread
 */

#include "weapon.hpp"
#include <cmath>
#include <algorithm>

// Check if weapon can use bullet type (override in subclasses)
// Inputs: BulletType | Outputs: bool
bool Weapon::CanUseBulletType(BulletType) const {
    return true;  // Default: can use all
}

// Basic weapon fire - single bullet
// Inputs: Position, BulletType, int damage | Outputs: std::vector<Bullet>
std::vector<Bullet> BasicWeapon::Fire(Position player_pos, BulletType bullet_type, int damage) {
    std::vector<Bullet> bullets;
    Position pos = {player_pos.x + 1, player_pos.y - 1};
    bullets.push_back(CreateBasicBullet(pos, bullet_type, damage));
    return bullets;
}

// Dual weapon fire - two bullets (left and right)
// Inputs: Position, BulletType, int damage | Outputs: std::vector<Bullet>
std::vector<Bullet> DualWeapon::Fire(Position player_pos, BulletType bullet_type, int damage) {
    std::vector<Bullet> bullets;
    // Left bullet
    Position left_pos = {player_pos.x, player_pos.y - 1};
    Bullet left = CreateBasicBullet(left_pos, bullet_type, damage);
    left.dx = -1;
    bullets.push_back(left);

    // Right bullet
    Position right_pos = {player_pos.x + 2, player_pos.y - 1};
    Bullet right = CreateBasicBullet(right_pos, bullet_type, damage);
    right.dx = 1;
    bullets.push_back(right);

    return bullets;
}

// Tri weapon fire - three bullets (spread)
// Inputs: Position, BulletType, int damage | Outputs: std::vector<Bullet>
std::vector<Bullet> TriWeapon::Fire(Position player_pos, BulletType bullet_type, int damage) {
    std::vector<Bullet> bullets;

    // Center bullet
    Position center_pos = {player_pos.x + 1, player_pos.y - 1};
    Bullet center = CreateBasicBullet(center_pos, bullet_type, damage);
    center.dx = 0;
    bullets.push_back(center);

    // Left bullet
    Position left_pos = {player_pos.x, player_pos.y - 1};
    Bullet left = CreateBasicBullet(left_pos, bullet_type, damage);
    left.dx = -1;
    bullets.push_back(left);

    // Right bullet
    Position right_pos = {player_pos.x + 2, player_pos.y - 1};
    Bullet right = CreateBasicBullet(right_pos, bullet_type, damage);
    right.dx = 1;
    bullets.push_back(right);

    return bullets;
}

// Explosive weapon fire - single explosive bullet
// Inputs: Position, BulletType, int damage | Outputs: std::vector<Bullet>
std::vector<Bullet> ExplosiveWeapon::Fire(Position player_pos, BulletType, int damage) {
    std::vector<Bullet> bullets;
    Position pos = {player_pos.x + 1, player_pos.y - 1};
    bullets.push_back(CreateExplosiveBullet(pos, damage));
    return bullets;
}

bool ExplosiveWeapon::CanUseBulletType(BulletType type) const {
    return type == BulletType::EXPLOSIVE;  // Only explosive bullets
}

std::unique_ptr<Weapon> CreateWeapon(WeaponType type) {
    switch (type) {
        case WeaponType::BASIC:
            return std::make_unique<BasicWeapon>();
        case WeaponType::DUAL:
            return std::make_unique<DualWeapon>();
        case WeaponType::TRI:
            return std::make_unique<TriWeapon>();
        case WeaponType::EXPLOSIVE:
            return std::make_unique<ExplosiveWeapon>();
        default:
            return std::make_unique<BasicWeapon>();
    }
}