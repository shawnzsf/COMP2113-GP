#pragma once
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/component.hpp"
#include "../core/types.hpp"
#include "../data/highscore.hpp"

class MenuRenderer {
public:
    MenuRenderer(HighScore& highscore);

    // Render main menu
    ftxui::Element RenderMainMenu(int selected_item);

    // Render scoreboard
    ftxui::Element RenderScoreboard();

    // Render controls page
    ftxui::Element RenderControls();

    // Render game over screen
    ftxui::Element RenderGameOver(int score, int wave);

    // Render difficulty selection
    ftxui::Element RenderDifficultySelect(int selected_difficulty);

private:
    HighScore& highscore;
};