#include "game.hpp"
#include "../data/highscore.hpp"
#include "../ui/hud.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/canvas.hpp"

#include <atomic>
#include <chrono>
#include <thread>
#include <string>

// Change the bullet icon to unicode
// Add an hud that shows the player's current weapon, score, wave, and cash. Also show cooldowns for abilities and weapon upgrades. The hud should be placed at the top of the screen and should be visually distinct from the gameplay area. This allows players to easily keep track of their status and make informed decisions during gameplay. For example, if the player sees that their current weapon is on cooldown, they might choose to use an ability or switch to a different weapon if they have one available. Additionally, showing the player's cash can help them decide when to visit the shop and what items or upgrades they can afford. Overall, a well-designed hud enhances the player's experience by providing important information at a glance without cluttering the screen.
// The current pasuse menu can also be increased, since a more comprehensive shopping system will be implemented, make the pause menu a full shop menu, where players can buy weapons, upgrades, and abilities. The shop should have a clear layout that categorizes items (e.g., weapons, upgrades, abilities) and displays the player's current cash and the cost of each item. Players should be able to navigate the shop using the arrow keys and select items to purchase with the Enter key. Additionally, there should be an option to exit the shop and resume the game. This allows players to make strategic decisions about their purchases without feeling rushed, as they can take their time to browse through the available options and plan their upgrades based on their current needs in the game.
// add weapon system, switch between different weapons, weapon can be upgraded, fire rate, etc. some amount of money to unlock weapon then upgrade
// Create a bullet system. Remove the explosive weapon, it should belong to the bullet system. We will have three types of bullets: basic, explosive (like the current explosive weapon), and piercing (laser-like, fast and can penetrate enemies). Each bullet type has different behavior and can be upgraded separately. For example, basic bullets can be upgraded to increase damage, explosive bullets can be upgraded to increase blast radius, and piercing bullets can be upgraded to penetrate more enemies. This adds more depth and strategy to the gameplay, as players can choose which bullet type to focus on based on their playstyle and the current wave of enemies. Player should be offered the option to upgrade specific property of the bullet, for example, For explosive bullet, you can choose to upgrade blast radius or damage. For piercing bullet, you can choose to upgrade penetration or speed. This allows for more customization and strategic choices in how players want to enhance their weapons.
// item system, temporarily increase the player's speed, or increase health, etc. One time use, or last for a few seconds. Can also be bought in the shop.
// ability system, abilities should also be bought and upgraded, including stuff like shield, or freeze the enemies
// Improve the spawning system; for each wave, you have a fixed level of toughness (e.g. for wave 1 is 5), and each regular (1)/elite (3)/boss (6) contribute differently to that toughness. For example, earlier waves can only have regular/elite enemies that could contribute to the overall toughness. In later waves boss will also be available for filling the toughness. And the toughness will increase over time. This ensures the waves feel more balanced and challenging as you progress.
// The overall difficulty should increase over time, for every like 5 or 10 waves, increase the enemy strength, so that the same enemies will have more health (boss also having higher damage) and give more points. This ensures the game remains challenging even if you have good weapons and upgrades.

enum class GameState {
    MainMenu,
    Playing,
    Scoreboard,
    GameOver
};

int main() {
    using namespace ftxui;

    Game game;
    HighScore highscore;
    HUD hud;
    GameState current_state = GameState::MainMenu;
    int selected_menu_item = 0;
    std::string player_name = "Player";
    bool paused = false;
    int shop_selected_item = 0;
    const std::vector<std::string> shop_items = {
        "Dual Shot - $50",
        "Tri Shot - $100",
        "Explosive Orb - $150",
        "Shield - $75",
        "Resume"
    };

    auto screen = ScreenInteractive::Fullscreen();

    // Main menu renderer
    auto main_menu_renderer = Renderer([&] {
        std::vector<std::string> menu_items = {"Start Game", "Scoreboard"};

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
            if (static_cast<int>(i) == selected_menu_item) {
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
    });

    // Scoreboard renderer
    auto scoreboard_renderer = Renderer([&] {
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
    });

    // Game renderer
    auto game_renderer = Renderer([&] {
        Canvas c(Game::WIDTH, Game::HEIGHT);
        game.Draw(c);

        Element game_display = canvas(c) | border | color(Color::White);

        // Create the HUD display at the top
        Element hud_display = hud.Render(game, game.GetShootCooldown(), game.GetMaxShootCooldown());

        // Stack HUD on top of game display
        Element combined_display = vbox({
            hud_display | size(HEIGHT, LESS_THAN, 5),
            separator(),
            game_display | flex_grow,
        });

        if (paused) {
            Elements shop_lines;
            shop_lines.push_back(text("SHOP MENU") | bold | color(Color::Yellow) | center);
            shop_lines.push_back(text("Cash: $" + std::to_string(game.GetCash())) | color(Color::Green) | center);
            shop_lines.push_back(text("Current Weapon: " + std::string(
                game.GetWeaponType() == WeaponType::BASIC ? "Basic" :
                game.GetWeaponType() == WeaponType::DUAL ? "Dual" :
                game.GetWeaponType() == WeaponType::TRI ? "Tri" : "Explosive"
            )) | color(Color::White) | center);
            shop_lines.push_back(text("Shield: " + std::string(game.HasShield() ? "Owned" : "Available")) | color(Color::White) | center);
            shop_lines.push_back(separator());

            for (size_t i = 0; i < shop_items.size(); ++i) {
                auto item_text = text((static_cast<int>(i) == shop_selected_item ? "> " : "  ") + shop_items[i]);
                if (static_cast<int>(i) == shop_selected_item) {
                    item_text = item_text | color(Color::Yellow) | bold;
                } else {
                    item_text = item_text | color(Color::White);
                }
                shop_lines.push_back(item_text);
            }

            shop_lines.push_back(text("") );
            shop_lines.push_back(text("Use ↑↓ to select, Enter to buy/resume, P to close") | color(Color::GrayLight) | center);

            auto shop_box = vbox(std::move(shop_lines)) | border | color(Color::Green);
            combined_display = vbox({
                combined_display,
                separator(),
                shop_box,
            });
        } else if (game.IsGameOver()) {
            combined_display = dbox({
                combined_display,
                filler() | size(HEIGHT, EQUAL, 1),
            });
        }

        return combined_display;
    });

    // Main renderer that switches based on state
    auto main_renderer = Renderer([&] {
        switch (current_state) {
            case GameState::MainMenu:
                return main_menu_renderer->Render();
            case GameState::Scoreboard:
                return scoreboard_renderer->Render();
            case GameState::Playing:
            case GameState::GameOver:
                return game_renderer->Render();
            default:
                return text("Unknown state") | center;
        }
    });

    // Event handler
    auto component = main_renderer | CatchEvent([&](Event event) {
        switch (current_state) {
            case GameState::MainMenu: {
                if (event == Event::ArrowUp) {
                    selected_menu_item = (selected_menu_item - 1 + 2) % 2;
                    return true;
                }
                if (event == Event::ArrowDown) {
                    selected_menu_item = (selected_menu_item + 1) % 2;
                    return true;
                }
                if (event == Event::Return) {
                    if (selected_menu_item == 0) { // Start Game
                        current_state = GameState::Playing;
                        paused = false;
                        shop_selected_item = 0;
                        game = Game(); // Reset game
                    } else if (selected_menu_item == 1) { // Scoreboard
                        current_state = GameState::Scoreboard;
                    }
                    return true;
                }
                if (event == Event::Escape) {
                    screen.ExitLoopClosure()();
                    return true;
                }
                break;
            }

            case GameState::Scoreboard: {
                if (event == Event::Escape) {
                    current_state = GameState::MainMenu;
                    paused = false;
                    return true;
                }
                break;
            }

            case GameState::Playing: {
                if (event == Event::Character('q') || event == Event::Character('Q')) {
                    // Save progress before returning to the menu
                    if (highscore.IsHighScore(game.GetScore())) {
                        highscore.AddScore(player_name, game.GetScore(), game.GetWave());
                    }
                    current_state = GameState::MainMenu;
                    paused = false;
                    return true;
                }

                if (event == Event::Character('p') || event == Event::Character('P')) {
                    paused = !paused;
                    return true;
                }

                if (paused) {
                    if (event == Event::ArrowUp) {
                        shop_selected_item = (shop_selected_item - 1 + static_cast<int>(shop_items.size())) % static_cast<int>(shop_items.size());
                        return true;
                    }
                    if (event == Event::ArrowDown) {
                        shop_selected_item = (shop_selected_item + 1) % static_cast<int>(shop_items.size());
                        return true;
                    }
                    if (event == Event::Return) {
                        switch (shop_selected_item) {
                            case 0: // Dual Shot
                                game.BuyWeapon(WeaponType::DUAL, 50);
                                break;
                            case 1: // Tri Shot
                                game.BuyWeapon(WeaponType::TRI, 100);
                                break;
                            case 2: // Explosive Orb
                                game.BuyWeapon(WeaponType::EXPLOSIVE, 150);
                                break;
                            case 3: // Shield
                                game.BuyShield(75);
                                break;
                            case 4: // Resume
                                paused = false;
                                break;
                        }
                        return true;
                    }
                    if (event == Event::Escape) {
                        paused = false;
                        return true;
                    }
                    return true;
                }

                bool handled = game.HandleEvent(event);

                if (game.IsGameOver()) {
                    current_state = GameState::GameOver;
                    // Save score if it's a high score
                    if (highscore.IsHighScore(game.GetScore())) {
                        highscore.AddScore(player_name, game.GetScore(), game.GetWave());
                    }
                }

                return handled;
            }

            case GameState::GameOver: {
                if (event == Event::Character('q') || event == Event::Character('Q') ||
                    event == Event::Escape) {
                    current_state = GameState::MainMenu;
                    return true;
                }
                return game.HandleEvent(event); // Still allow some game input
            }
        }

        return false;
    });

    std::atomic<bool> running = true;
    std::thread ticker([&] {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
            if (current_state == GameState::Playing && !paused) {
                screen.Post([&] {
                    game.Update();
                    screen.RequestAnimationFrame();
                });
            }
        }
    });

    // Run the interactive loop
    screen.Loop(component);

    running = false;
    ticker.join();

    return 0;
}
