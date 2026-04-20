/**
 * @file hud.cpp
 * @brief Heads-Up Display rendering
 */

#include "hud.hpp"
#include "ftxui/dom/elements.hpp"
#include <sstream>
#include <iomanip>

using namespace ftxui;

ftxui::Element HUD::Render(const Game& game, int shoot_cooldown, int max_shoot_cooldown) {
    Elements hud_content;

    // Create six columns: Weapon | Score | Wave | Cash | Enemies | Health
    hud_content.push_back(hbox({
        RenderWeaponInfo(game) | flex,
        separator(),
        RenderScoreInfo(game) | flex,
        separator(),
        RenderWaveInfo(game) | flex,
        separator(),
        RenderCashInfo(game) | flex,
        separator(),
        RenderEnemyInfo(game) | flex,
        separator(),
        RenderHealthInfo(game) | flex,
    }) | color(Color::White));


    hud_content.push_back(RenderCooldownBar(shoot_cooldown, max_shoot_cooldown, "Weapon Cooldown")
                         | color(Color::Yellow));

    hud_content.push_back(text(""));  // Spacer

    // Add random event display - always show event bar at bottom
    std::string event_bar = "══════ WAVE " + std::to_string(game.GetWave()) + " EVENT: ";
    if (game.HasActiveEvent()) {
        const RandomEvent& evt = game.GetCurrentEvent();
        if (!evt.name.empty()) {
            event_bar += evt.name + " - " + evt.description;
        } else {
            event_bar += "Clear Skies";
        }
    } else {
        event_bar += "Clear Skies";
    }
    event_bar += " ══════";
    hud_content.push_back(text(event_bar) | color(Color::Magenta) | center);

    return vbox(std::move(hud_content)) 
        | border 
        | color(Color::Cyan) 
        | bgcolor(Color::RGB(20, 30, 40));
}

ftxui::Element HUD::RenderWeaponInfo(const Game& game) const {
    std::string weapon_name;
    switch (game.GetWeaponType()) {
        case WeaponType::BASIC:
            weapon_name = "Basic Pistol";
            break;
        case WeaponType::DUAL:
            weapon_name = "Dual Shot";
            break;
        case WeaponType::TRI:
            weapon_name = "Tri Shot";
            break;
        case WeaponType::EXPLOSIVE:
            weapon_name = "Explosive Orb";
            break;
        default:
            weapon_name = "Unknown";
    }

    // Get bullet type info
    std::string bullet_name;
    Color bullet_color;
    switch (game.GetBulletType()) {
        case BulletType::NORMAL:
            bullet_name = "Basic";
            bullet_color = Color::White;
            break;
        case BulletType::EXPLOSIVE:
            bullet_name = "Explosive";
            bullet_color = Color::Red;
            break;
        case BulletType::PIERCING:
            bullet_name = "Piercing";
            bullet_color = Color::Cyan;
            break;
        default:
            bullet_name = "Unknown";
            bullet_color = Color::GrayDark;
    }

    // Show unlock status
    std::string unlock_info = "";
    if (!game.HasExplosive() && !game.HasPiercing()) {
        unlock_info = "";
    }

    Elements weapon_info;
    weapon_info.push_back(text("WEAPON") | bold | color(Color::Yellow));
    weapon_info.push_back(text(weapon_name) | color(Color::Green));
    weapon_info.push_back(text("[" + bullet_name + "]") | color(bullet_color) | bold);
    if (!game.HasExplosive() || !game.HasPiercing()) {
        std::string unlocks = "";
        if (!game.HasExplosive()) unlocks += "E";
        if (!game.HasPiercing()) unlocks += "P";
        weapon_info.push_back(text("(!" + unlocks + ")") | color(Color::GrayDark));
    }
    if (game.HasShield()) {
        weapon_info.push_back(text("Shield Active") | color(Color::Blue));
    }
    if (game.GetSpeedBoostTimer() > 0) {
        weapon_info.push_back(text("SPD UP") | color(Color::Green));
    }
    if (game.GetDamageBoostTimer() > 0) {
        weapon_info.push_back(text("DMG UP") | color(Color::Red));
    }
    if (game.GetFreezeTimer() > 0) {
        weapon_info.push_back(text("ENEMIES FROZEN") | color(Color::Cyan));
    }

    return vbox(std::move(weapon_info)) | center;
}

ftxui::Element HUD::RenderScoreInfo(const Game& game) const {
    Elements score_info;
    score_info.push_back(text("SCORE") | bold | color(Color::Yellow));
    score_info.push_back(text(std::to_string(game.GetScore())) | color(Color::Magenta) | bold);
    
    return vbox(std::move(score_info)) | center;
}

ftxui::Element HUD::RenderWaveInfo(const Game& game) const {
    Elements wave_info;
    wave_info.push_back(text("WAVE") | bold | color(Color::Yellow));
    wave_info.push_back(text(std::to_string(game.GetWave())) | color(Color::Cyan) | bold);
    
    return vbox(std::move(wave_info)) | center;
}

ftxui::Element HUD::RenderCashInfo(const Game& game) const {
    Elements cash_info;
    cash_info.push_back(text("CASH") | bold | color(Color::Yellow));
    cash_info.push_back(text("$" + std::to_string(game.GetCash())) | color(Color::Green) | bold);
    
    return vbox(std::move(cash_info)) | center;
}

ftxui::Element HUD::RenderCooldownBar(int current, int max, const std::string& label) const {
    if (max <= 0) return text("");
    
    // Create a simple text-based cooldown indicator
    int bar_width = 20;
    int filled = max > 0 ? (bar_width * (max - current)) / max : bar_width;
    filled = std::max(0, std::min(filled, bar_width));
    
    std::string bar;
    for (int i = 0; i < bar_width; ++i) {
        bar += (i < filled) ? "█" : "░";
    }
    
    Elements cooldown_display;
    cooldown_display.push_back(text(label + ": ") | bold);
    cooldown_display.push_back(text(bar) | color(current > 0 ? Color::Red : Color::Green));
    
    std::ostringstream oss;
    oss << " (" << current << "/" << max << ")";
    cooldown_display.push_back(text(oss.str()) | color(Color::White));
    
    return hbox(std::move(cooldown_display)) | center;
}

ftxui::Element HUD::RenderEnemyInfo(const Game& game) const {
    Elements enemy_info;
    enemy_info.push_back(text("👾 ENEMIES") | bold | color(Color::Yellow));
    enemy_info.push_back(text(std::to_string(game.GetEnemyCount())) | color(Color::Red) | bold);
    
    return vbox(std::move(enemy_info)) | center;
}

ftxui::Element HUD::RenderHealthInfo(const Game& game) const {
    int health = game.GetPlayerHealth();
    int max_health = game.GetPlayerMaxHealth();
    
    // Create a simple health bar
    std::string health_bar;
    for (int i = 0; i < max_health; ++i) {
        health_bar += (i < health) ? "❤" : "🖤";
    }
    
    Elements health_info;
    health_info.push_back(text("HP") | bold | color(Color::Yellow));
    health_info.push_back(text(health_bar) | color(health > 1 ? Color::Green : Color::Red));

    return vbox(std::move(health_info)) | center;
}

ftxui::Element HUD::RenderEventInfo(const Game& game) const {
    const RandomEvent& event = game.GetCurrentEvent();

    Color event_color;
    std::string prefix;
    switch (event.type) {
        case EventType::PLAYER_BUFF:
            event_color = Color::Green;
            prefix = "✨";
            break;
        case EventType::ENEMY_DEBUFF:
            event_color = Color::Cyan;
            prefix = "💫";
            break;
        case EventType::ENEMY_BUFF:
            event_color = Color::Red;
            prefix = "⚠";
            break;
        case EventType::PLAYER_DEBUFF:
            event_color = Color::Red;
            prefix = "💢";
            break;
        default:
            event_color = Color::White;
            prefix = "";
    }

    std::string event_text = prefix + " " + event.name + ": " + event.description;

    return hbox({
        text("[" + event.name + "] ") | bold | color(event_color),
        text(event.description) | color(event_color)
    }) | center;
}
