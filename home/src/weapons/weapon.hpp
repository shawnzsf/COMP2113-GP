#pragma once
#include "ftxui/dom/canvas.hpp"
#include <vector>
#include <memory>
#include "../entities/bullet.hpp"
#include "../entities/enemy.hpp"

enum class WeaponType {
    BASIC,
    DUAL,
    TRI,
    EXPLOSIVE
};

class Weapon {
public:
    virtual ~Weapon() = default;
    virtual std::vector<Bullet> Fire(Position player_pos, BulletType bullet_type, int damage) = 0;
    virtual std::string GetName() const = 0;
    virtual bool CanUseBulletType(BulletType type) const;
};

class BasicWeapon : public Weapon {
public:
    std::vector<Bullet> Fire(Position player_pos, BulletType bullet_type, int damage) override;
    std::string GetName() const override { return "Basic"; }
};

class DualWeapon : public Weapon {
public:
    std::vector<Bullet> Fire(Position player_pos, BulletType bullet_type, int damage) override;
    std::string GetName() const override { return "Dual"; }
};

class TriWeapon : public Weapon {
public:
    std::vector<Bullet> Fire(Position player_pos, BulletType bullet_type, int damage) override;
    std::string GetName() const override { return "Tri"; }
};

class ExplosiveWeapon : public Weapon {
public:
    std::vector<Bullet> Fire(Position player_pos, BulletType bullet_type, int damage) override;
    std::string GetName() const override { return "Explosive"; }
    bool CanUseBulletType(BulletType type) const override;
};

// Weapon factory
using WeaponPtr = std::unique_ptr<Weapon>;
WeaponPtr CreateWeapon(WeaponType type);