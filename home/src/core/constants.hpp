#pragma once

// Game dimensions
namespace GameConstants {
    constexpr int WIDTH = 170;
    constexpr int HEIGHT = 125;

    // Player settings
    constexpr int PLAYER_MAX_HEALTH = 3;
    constexpr int PLAYER_MOVE_SPEED = 2;
    constexpr int PLAYER_WIDTH = 3;
    constexpr int PLAYER_HEIGHT = 2;

    // Bullet settings
    constexpr int BULLET_MOVE_SPEED = 2;
    constexpr int BULLET_DEFAULT_DAMAGE = 1;
    constexpr int SHOOT_COOLDOWN = 6;

    // Enemy settings
    constexpr int ENEMY_MOVE_SPEED = 1;
    constexpr int ENEMY_MOVE_INTERVAL = 15;
    constexpr int ENEMY_DESCENT_INTERVAL = 30;
    constexpr int ENEMY_SPAWN_COUNT = 10;
    constexpr int COLLISION_RADIUS = 1;

    // Points
    constexpr int POINTS_REGULAR_ENEMY = 10;
    constexpr int POINTS_ELITE_ENEMY = 30;
    constexpr int POINTS_BOSS_ENEMY = 100;
    constexpr int POINTS_DROPSHIP_ENEMY = 500;
}