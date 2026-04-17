#pragma once
#include "ftxui/dom/elements.hpp"
#include "../core/game.hpp"
#include <string>

class HUD {
public:
    HUD() = default;

    // Render the HUD display with game information
    ftxui::Element Render(const Game& game, int shoot_cooldown, int max_shoot_cooldown);

private:
    // Helper methods to format individual HUD components
    ftxui::Element RenderWeaponInfo(const Game& game) const;
    ftxui::Element RenderScoreInfo(const Game& game) const;
    ftxui::Element RenderWaveInfo(const Game& game) const;
    ftxui::Element RenderCashInfo(const Game& game) const;
    ftxui::Element RenderEnemyInfo(const Game& game) const;
    ftxui::Element RenderHealthInfo(const Game& game) const;
    ftxui::Element RenderCooldownBar(int current, int max, const std::string& label) const;
};
