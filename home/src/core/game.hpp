#pragma once
#include "ftxui/dom/canvas.hpp"
#include "ftxui/component/event.hpp"
#include "../entities/enemy.hpp"
#include "../entities/player.hpp"
#include <vector>
#include <string>

enum class WeaponType {
    BASIC,
    DUAL,
    TRI,
    EXPLOSIVE
};

// Random event system
enum class EventType {
    PLAYER_BUFF,     // Positive player effect
    ENEMY_DEBUFF,    // Negative enemy effect
    ENEMY_BUFF,      // Positive enemy effect (harder)
    PLAYER_DEBUFF    // Negative player effect
};

struct RandomEvent {
    std::string name;
    std::string description;
    EventType type;
    int duration;  // Duration in seconds, 0 = instant/until next wave
};

class Game {
public:
    Game();

    void Update();                    // Game logic (called every frame)
    void Draw(ftxui::Canvas& canvas); // Draw on canvas
    bool HandleEvent(const ftxui::Event& event);  // Returns true if event was consumed

    bool IsGameOver() const;
    int GetScore() const;
    int GetWave() const;
    bool CanAfford(int amount) const;
    bool BuyWeapon(WeaponType type, int cost);
    bool BuyShield(int cost);
    int GetCash() const;
    WeaponType GetWeaponType() const;
    void SetWeaponType(WeaponType type);
    bool HasShield() const;
    int GetShootCooldown() const;
    int GetMaxShootCooldown() const;
    int GetEnemyCount() const;
    int GetPlayerHealth() const;
    int GetPlayerMaxHealth() const;
    
    // Upgrade accessors
    bool HasRapidFire() const;
    bool HasTimeSlow() const;
    int GetFireRateUpgrades() const;
    int GetDamageUpgrades() const;
    int GetSpeedUpgrades() const;
    int GetBulletSpeedUpgrades() const;
    int GetClipSizeUpgrades() const;

    // Bullet type accessors
    BulletType GetBulletType() const;
    void SetBulletType(BulletType type);
    bool CanUseBulletType(BulletType type) const;
    bool HasExplosive() const;
    bool HasPiercing() const;

    // Purchase methods for bullet unlocks and upgrades
    bool BuyExplosiveBullet(int cost);
    bool BuyPiercingBullet(int cost);
    bool UpgradeBasicBulletDamage(int cost);
    bool UpgradeBasicBulletSpeed(int cost);
    bool UpgradeExplosiveDamage(int cost);
    bool UpgradeExplosiveRadius(int cost);
    bool UpgradePiercingDamage(int cost);
    bool UpgradePiercingPenetration(int cost);

    // Item purchase methods
    bool BuySpeedBoost(int cost);
    bool BuyHealthPack(int cost);
    bool BuyShieldPack(int cost);
    bool BuyDamageBoost(int cost);

    // Ability purchase methods
    bool BuyRapidFire(int cost);
    bool BuyTimeSlow(int cost);
    bool BuyFreeze(int cost);

    // Activate abilities
    void ActivateTimeSlow();
    void ActivateFreeze();

    // Item/ability accessors
    int GetSpeedBoostTimer() const;
    int GetDamageBoostTimer() const;
    bool HasFreezeAbility() const;
    int GetTimeSlowTimer() const;
    int GetFreezeTimer() const;

    // Random event accessors
    const RandomEvent& GetCurrentEvent() const;
    bool HasActiveEvent() const;
    void GenerateRandomEvent();
    void ApplyEventEffect();

    static constexpr int WIDTH = 170;
    static constexpr int HEIGHT = 125;

private:
    // Game state
    Player player;
    std::vector<Bullet> player_bullets;
    std::vector<Enemy> enemies;
    
    int score = 0;
    int frame_count = 0;
    int wave = 1;
    bool game_over = false;
    
    // Game constants
    static constexpr int PLAYER_MOVE_SPEED = 1;  // Smaller steps for smoother feel
    static constexpr int BULLET_MOVE_SPEED = 2;
    static constexpr int ENEMY_MOVE_SPEED = 1;
    static constexpr int ENEMY_MOVE_INTERVAL = 15;
    static constexpr int ENEMY_DESCENT_INTERVAL = 30;
    static constexpr int ENEMY_SPAWN_COUNT = 10;
    static constexpr int COLLISION_RADIUS = 1;
    static constexpr int POINTS_REGULAR_ENEMY = 10;
    static constexpr int POINTS_ELITE_ENEMY = 30;
    static constexpr int POINTS_BOSS_ENEMY = 100;
    static constexpr int POINTS_DROPSHIP_ENEMY = 500;

    // Get difficulty multiplier (increases every 5 waves)
    int GetDifficultyMultiplier() const { return 1 + (wave / 5); }
    int GetDifficultyLevel() const { return wave / 5; }

    static constexpr int SHOOT_COOLDOWN = 6;  // Frames between shots
    static constexpr int SHOOT_HOLD_FRAMES = 10; // Keep shooting active briefly after the last event
    static constexpr int INPUT_HOLD_FRAMES = 6;  // Keep a direction active briefly after the last event
    int shoot_cooldown = 0;
    int shoot_hold_timer = 0;

    // Input state
    bool move_left = false;
    bool move_right = false;
    bool move_up = false;
    bool move_down = false;
    int move_left_timer = 0;
    int move_right_timer = 0;
    int move_up_timer = 0;
    int move_down_timer = 0;
    bool shoot_requested = false;

    // Player upgrades and currency
    int cash = 0;
    WeaponType weapon_type = WeaponType::BASIC;
    bool has_dual_weapon = false;
    bool has_tri_weapon = false;
    
    // Upgrade tracking
    bool has_rapid_fire = false;
    bool has_time_slow = false;
    int fire_rate_upgrades = 0;
    int damage_upgrades = 0;
    int speed_upgrades = 0;
    int bullet_speed_upgrades = 0;
    int clip_size_upgrades = 0;

    // Bullet system state
    BulletType current_bullet_type = BulletType::NORMAL;
    bool owned_explosive = false;
    bool owned_piercing = false;

    // Bullet upgrade stats
    int basic_bullet_damage = 1;
    int explosive_bullet_damage = 1;
    int explosive_bullet_radius = 1;
    int piercing_bullet_damage = 2;
    int piercing_bullet_penetration = 2;
    int piercing_bullet_speed = 4;

    // Item timers (in frames, 60 frames = 1 second)
    int speed_boost_timer = 0;
    int damage_boost_timer = 0;
    int shield_pack_timer = 0;
    int time_slow_timer = 0;
    int freeze_timer = 0;

    // Ability upgrades
    bool has_freeze = false;
    int rapid_fire_level = 0;  // 0 = not owned, 1+ = level
    int shield_level = 0;

    // Random event state
    RandomEvent current_event;
    int event_timer = 0;
    bool current_wave_has_event = false;

    // Private methods
    void MoveEnemies();
    void MoveBullets();
    void CheckCollisions();
    void SpawnEnemies();
    void UpdateWave();
    void FireWeapon();
};
