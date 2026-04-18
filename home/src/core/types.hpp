#pragma once

// Game state for main menu navigation
enum class GameState {
    MainMenu,
    Playing,
    Scoreboard,
    Controls,
    GameOver,
    DifficultySelect
};

// Difficulty level for game balance
enum class DifficultyLevel {
    Easy,
    Medium,
    Hard
};