/**
 * @file menu.cpp
 * @brief Menu rendering implementation
 *
 * Handles main menu, scoreboard, controls, game over,
 * and difficulty selection rendering.
 */

#include "menu.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/canvas.hpp"

using namespace ftxui;

// Constructor - initialize with high score reference
// Inputs: HighScore& highscore | Outputs: None
MenuRenderer::MenuRenderer(HighScore& highscore) : highscore(highscore) {}

// Render main menu
// Inputs: int selected_item | Outputs: Element (FTXUI renderable)
Element MenuRenderer::RenderMainMenu(int selected_item) {
    std::vector<std::string> menu_items = {"Start Game", "Scoreboard", "Controls"};

    Elements menu_elements;
    menu_elements.push_back(text("  ███████╗██████╗  █████╗  ██████╗███████╗    ███████╗██╗  ██╗ ██████╗  ██████╗ ████████╗███████╗██████╗ ") | bold | color(Color::Red) | center);
    menu_elements.push_back(text("  ██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝    ██╔════╝██║  ██║██╔═══██╗██╔═══██╗╚══██╔══╝██╔════╝██╔══██╗") | bold | color(Color::Red) | center);
    menu_elements.push_back(text("  ███████╗██████╔╝███████║██║     █████╗      ███████╗███████║██║   ██║██║   ██║   ██║   █████╗  ██████╔╝") | bold | color(Color::Red) | center);
    menu_elements.push_back(text("  ╚════██║██╔═══╝ ██╔══██║██║     ██╔══╝      ╚════██║██╔══██║██║   ██║██║   ██║   ██║   ██╔══╝  ██╔══██╗") | bold | color(Color::Red) | center);
    menu_elements.push_back(text("  ███████║██║     ██║  ██║╚██████╗███████╗    ███████║██║  ██║╚██████╔╝╚██████╔╝   ██║   ███████╗██║  ██║") | bold | color(Color::Red) | center);
    menu_elements.push_back(text("  ╚══════╝╚═╝     ╚═╝  ╚═╝ ╚═════╝╚══════╝    ╚══════╝╚═╝  ╚═╝ ╚═════╝  ╚═════╝    ╚═╝   ╚══════╝╚═╝  ╚═╝") | bold | color(Color::Red) | center);
    menu_elements.push_back(text(""));

    for (size_t i = 0; i < menu_items.size(); ++i) {
        auto button = text("    " + menu_items[i] + "    ") | bold | center;
        if (static_cast<int>(i) == selected_item) {
            button = button | bgcolor(Color::Blue) | color(Color::White) | bold;
        } else {
            button = button | bgcolor(Color::DarkBlue) | color(Color::White);
        }
        menu_elements.push_back(button | hcenter);
        if (i + 1 < menu_items.size()) {
            menu_elements.push_back(text(""));
        }
    }

    menu_elements.push_back(text(""));
    menu_elements.push_back(text("Use ↑↓ to navigate, Enter to select") | color(Color::GrayLight) | center);
    menu_elements.push_back(text("ESC to quit") | color(Color::GrayLight) | center);

    return vbox(std::move(menu_elements)) | center | border | color(Color::White) | bgcolor(Color::RGB(16, 16, 16));
}

// Render scoreboard
// Inputs: None | Outputs: Element
Element MenuRenderer::RenderScoreboard() {
    const auto& top_scores = highscore.GetTopScores();

    Elements scoreboard_elements;
    scoreboard_elements.push_back(text("HIGH SCORES") | bold | color(Color::Green) | center);
    scoreboard_elements.push_back(separator());
    scoreboard_elements.push_back(text(""));

    if (top_scores.empty()) {
        scoreboard_elements.push_back(text("No scores yet!") | color(Color::GrayLight) | center);
    } else {
        for (size_t i = 0; i < top_scores.size(); ++i) {
            const auto& entry = top_scores[i];
            std::string rank = std::to_string(i + 1) + ".";
            std::string score_line = rank + " " + entry.player_name + " - " +
                                     std::to_string(entry.score) + " (Wave " +
                                     std::to_string(entry.wave) + ")";
            scoreboard_elements.push_back(text(score_line) | color(Color::White));
        }
    }

    scoreboard_elements.push_back(text(""));
    scoreboard_elements.push_back(text("Press ESC to return to menu") | color(Color::GrayLight) | center);

    return vbox(std::move(scoreboard_elements)) | center | border | color(Color::White);
}

// Render controls page
// Inputs: None | Outputs: Element
Element MenuRenderer::RenderControls() {
    Elements controls_elements;
    controls_elements.push_back(text("CONTROLS & GAMEPLAY") | bold | color(Color::Yellow) | center);
    controls_elements.push_back(separator());
    controls_elements.push_back(text(""));

    controls_elements.push_back(text("Gameplay Overview:") | bold | color(Color::Cyan));
    controls_elements.push_back(text("- Survive waves of enemies while upgrading weapons and bullets.") | color(Color::GrayLight));
    controls_elements.push_back(text("- Press P during play to open the Shop and upgrade/buy items.") | color(Color::GrayLight));
    controls_elements.push_back(text("- Press Q during play to quit back to the main menu.") | color(Color::GrayLight));
    controls_elements.push_back(text(""));

    controls_elements.push_back(text("Movement & Shooting:") | bold | color(Color::Cyan));
    controls_elements.push_back(text("- Use the keyboard to move the player ship and aim.") | color(Color::GrayLight));
    controls_elements.push_back(text("- Press SPACE to shoot.") | color(Color::GrayLight));
    controls_elements.push_back(text(""));

    controls_elements.push_back(text("Weapons & Bullets:") | bold | color(Color::Cyan));
    controls_elements.push_back(text("- Z/X/C: Switch between weapon types.") | color(Color::GrayLight));
    controls_elements.push_back(text("- 1/2/3: Switch bullet types (Basic, Explosive, Piercing).") | color(Color::GrayLight));
    controls_elements.push_back(text("- Explosive bullets cannot be used with Dual or Tri Shot weapons.") | color(Color::GrayLight));
    controls_elements.push_back(text(""));
    controls_elements.push_back(text("Weapon Types:") | bold | color(Color::Cyan));
    controls_elements.push_back(text("- Basic Pistol: single-shot damage.") | color(Color::GrayLight));
    controls_elements.push_back(text("- Dual Shot: fires two bullets at once.") | color(Color::GrayLight));
    controls_elements.push_back(text("- Tri Shot: fires three bullets in a spread.") | color(Color::GrayLight));
    controls_elements.push_back(text("- Explosive: area damage on impact.") | color(Color::GrayLight));
    controls_elements.push_back(text("- Piercing: bullets pass through enemies.") | color(Color::GrayLight));
    controls_elements.push_back(text(""));

    controls_elements.push_back(text("Shop Controls:") | bold | color(Color::Cyan));
    controls_elements.push_back(text("- ← / → : Switch shop categories.") | color(Color::GrayLight));
    controls_elements.push_back(text("- ↑ / ↓ : Select item.") | color(Color::GrayLight));
    controls_elements.push_back(text("- ENTER : Buy or upgrade selected item.") | color(Color::GrayLight));
    controls_elements.push_back(text("- P or ESC : Resume the game.") | color(Color::GrayLight));
    controls_elements.push_back(text(""));

    controls_elements.push_back(text("Press ESC to return to the main menu.") | color(Color::GrayLight));

    return vbox(std::move(controls_elements)) | center | border | color(Color::White);
}

// Render game over screen
// Inputs: int score, int wave | Outputs: Element
Element MenuRenderer::RenderGameOver(int score, int wave) {
    Elements go_elements;

    // ASCII Art header
    go_elements.push_back(text("") | size(HEIGHT, EQUAL, 2));
    go_elements.push_back(text("  ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ ") | bold | color(Color::Red) | center);
    go_elements.push_back(text(" ██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗") | bold | color(Color::Red) | center);
    go_elements.push_back(text(" ██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  ██████╔╝") | bold | color(Color::Red) | center);
    go_elements.push_back(text(" ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║██║   ██║██╔══╝  ██╔══██╗") | bold | color(Color::Red) | center);
    go_elements.push_back(text(" ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝╚██████╔╝███████╗██║  ██║") | bold | color(Color::Red) | center);
    go_elements.push_back(text("  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝  ╚═════╝ ╚══════╝╚═╝  ╚═╝") | bold | color(Color::Red) | center);
    go_elements.push_back(text("") | size(HEIGHT, EQUAL, 1));

    // Stats box
    go_elements.push_back(text("╔═══════════════════════════════════════════╗") | color(Color::Yellow) | center);
    go_elements.push_back(text("║                 SUMMARY                   ║") | bold | color(Color::Red) | center);
    go_elements.push_back(text("╠═══════════════════════════════════════════╣") | color(Color::Yellow) | center);

    std::string score_line = "║      FINAL SCORE:    " + std::to_string(score) + std::string(20 - std::to_string(score).length(), ' ') + " ║";
    go_elements.push_back(text(score_line) | color(Color::Cyan) | center);

    std::string wave_line = "║      WAVES SURVIVED: " + std::to_string(wave) + std::string(18 - std::to_string(wave).length(), ' ') + "   ║";
    go_elements.push_back(text(wave_line) | color(Color::Cyan) | center);

    go_elements.push_back(text("╚═══════════════════════════════════════════╝") | color(Color::Yellow) | center);
    go_elements.push_back(text("") | size(HEIGHT, EQUAL, 1));

    // Menu options
    go_elements.push_back(text("[ Q ] Return to Main Menu") | bold | color(Color::White) | center);
    go_elements.push_back(text("[ R ] Restart Game") | bold | color(Color::Yellow) | center);
    go_elements.push_back(text("") | size(HEIGHT, EQUAL, 2));

    return vbox(std::move(go_elements)) | center | color(Color::White) | bgcolor(Color::RGB(10, 10, 15));
}

// Render difficulty selection
// Inputs: int selected_difficulty | Outputs: Element
Element MenuRenderer::RenderDifficultySelect(int selected_difficulty) {
    Elements diff_elements;

    diff_elements.push_back(text("") | size(HEIGHT, EQUAL, 2));
    diff_elements.push_back(text("╔═══════════════════════════════════════════════════════════════════╗") | bold | color(Color::Cyan) | center);
    diff_elements.push_back(text("║             SELECT DIFFICULTY LEVEL                               ║") | bold | color(Color::Cyan) | center);
    diff_elements.push_back(text("╚═══════════════════════════════════════════════════════════════════╝") | bold | color(Color::Cyan) | center);
    diff_elements.push_back(text(""));

    const char* difficulties[] = {"EASY", "MEDIUM", "HARD"};
    const char* descriptions[] = {
        "+50% Cash     | -30% Enemy HP   | -30% Enemy Damage",
        "Normal Cash  | Normal Enemy HP | Normal Enemy Damage",
        "-30% Cash    | +50% Enemy HP   | +50% Enemy Damage"
    };
    const Color colors[] = {Color::Green, Color::Yellow, Color::Red};

    for (int i = 0; i < 3; ++i) {
        std::string prefix = (i == selected_difficulty) ? "→ " : "  ";
        if (i == selected_difficulty) {
            diff_elements.push_back(text(prefix + difficulties[i]) | bold | bgcolor(Color::Blue) | color(Color::White));
        } else {
            diff_elements.push_back(text(prefix + difficulties[i]) | color(colors[i]));
        }
        diff_elements.push_back(text("    " + std::string(descriptions[i])) | color(Color::GrayLight));
        diff_elements.push_back(text(""));
    }

    diff_elements.push_back(separator());
    diff_elements.push_back(text("↑↓ Select difficulty | ENTER to confirm | ESC to go back") | color(Color::GrayLight) | center);
    diff_elements.push_back(text(""));

    return vbox(std::move(diff_elements)) | center | border | color(Color::White) | bgcolor(Color::RGB(10, 10, 15));
}