#include "hud.hpp"
#include "ftxui/dom/elements.hpp"
#include <sstream>
#include <iomanip>

using namespace ftxui;

ftxui::Element HUD::Render(const Game& game, int shoot_cooldown, int max_shoot_cooldown) {
    Elements hud_content;

    // Create four columns: Weapon | Score | Wave | Cash
    hud_content.push_back(hbox({
        RenderWeaponInfo(game) | flex,
        separator(),
        RenderScoreInfo(game) | flex,
        separator(),
        RenderWaveInfo(game) | flex,
        separator(),
        RenderCashInfo(game) | flex,
    }) | color(Color::White));

    // Add weapon cooldown bar
    hud_content.push_back(RenderCooldownBar(shoot_cooldown, max_shoot_cooldown, "Weapon Cooldown") 
                         | color(Color::Yellow));

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
    
    Elements weapon_info;
    weapon_info.push_back(text("WEAPON") | bold | color(Color::Yellow));
    weapon_info.push_back(text(weapon_name) | color(Color::Green));
    if (game.HasShield()) {
        weapon_info.push_back(text("Shield Active") | color(Color::Blue));
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
