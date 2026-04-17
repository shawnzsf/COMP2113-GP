#include "game.hpp"
#include "../data/highscore.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/dom/canvas.hpp"

#include <atomic>
#include <chrono>
#include <thread>
#include <string>

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
            game_display = vbox({
                game_display,
                separator(),
                shop_box,
            });
        } else if (game.IsGameOver()) {
            game_display = dbox({
                game_display,
                filler() | size(HEIGHT, EQUAL, 1),
            });
        }

        return game_display;
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
