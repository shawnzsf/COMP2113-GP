#pragma once
#include "ftxui/dom/canvas.hpp"
#include <vector>
#include <string>

struct Position {
    int x;
    int y;
};

enum class BulletType {
    NORMAL,
    EXPLOSIVE,
    PIERCING
};

class Bullet {
public:
    Position pos;
    bool active = true;
    int dx = 0;
    int dy = -1;
    int length = 1;
    int lifetime = 0;
    int explode_timer = 0;
    BulletType type = BulletType::NORMAL;
    int damage = 1;
    int penetration = 0;  // For piercing bullets - remaining enemies to penetrate

    Bullet() = default;
    Bullet(Position start_pos, BulletType bullet_type, int bullet_damage = 1);

    void Update();
    void Draw(ftxui::Canvas& canvas) const;
    std::string GetSymbol() const;
    ftxui::Color GetColor() const;
};

// Factory functions for creating bullets
Bullet CreateBasicBullet(Position pos, BulletType type, int damage);
Bullet CreateExplosiveBullet(Position pos, int damage);
Bullet CreatePiercingBullet(Position pos, int damage, int penetration);