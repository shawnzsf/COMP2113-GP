#pragma once
#include "ftxui/dom/canvas.hpp"
#include "../entities/enemy.hpp"

class Player {
public:
    Player(int start_x = 0, int start_y = 0);

    void Update();
    void Draw(ftxui::Canvas& canvas) const;

    // Movement
    void MoveLeft();
    void MoveRight();
    void MoveUp();
    void MoveDown();
    void SetPosition(int x, int y);
    void SetBounds(int width, int height);

    // Combat
    void TakeDamage(int damage);
    bool IsAlive() const;

    // Shield mechanics
    void ActivateShield();
    void DeactivateShield();
    bool HasShield() const;
    void AbsorbHit();  // Shield absorbs a hit

    // Getters
    Position GetPosition() const;
    int GetHealth() const;
    int GetMaxHealth() const;

    // Constants
    static constexpr int MAX_HEALTH = 3;
    static constexpr int MOVE_SPEED = 2;
    static constexpr int WIDTH = 4;
    static constexpr int HEIGHT = 3;

private:
    Position pos;
    int health;
    bool shield_active;
    int shield_hits_remaining;  // How many hits the shield can absorb
    int screen_width;
    int screen_height;
};