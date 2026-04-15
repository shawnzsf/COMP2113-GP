#include "highscore.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>

HighScore::HighScore() : filename("../data/highscores.txt") {
    LoadScores();
}

HighScore::~HighScore() {
    SaveScores();
}

bool HighScore::LoadScores() {
    scores.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Parse format: "PlayerName Score Wave"
        size_t first_space = line.find(' ');
        size_t second_space = line.find(' ', first_space + 1);

        if (first_space != std::string::npos && second_space != std::string::npos) {
            try {
                std::string player_name = line.substr(0, first_space);
                int score = std::stoi(line.substr(first_space + 1, second_space - first_space - 1));
                int wave = std::stoi(line.substr(second_space + 1));

                scores.push_back({player_name, score, wave});
            } catch (const std::exception&) {
                // Skip invalid lines
                continue;
            }
        }
    }

    SortScores();
    return true;
}

bool HighScore::SaveScores() {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    for (const auto& entry : scores) {
        file << entry.player_name << " " << entry.score << " " << entry.wave << "\n";
    }

    return true;
}

void HighScore::AddScore(const std::string& player_name, int score, int wave) {
    scores.push_back({player_name, score, wave});
    SortScores();

    // Keep only top MAX_SCORES
    if (scores.size() > MAX_SCORES) {
        scores.resize(MAX_SCORES);
    }

    SaveScores();
}

const std::vector<ScoreEntry>& HighScore::GetTopScores() const {
    return scores;
}

bool HighScore::IsHighScore(int score) const {
    if (scores.size() < MAX_SCORES) {
        return true;
    }

    return score > scores.back().score;
}

void HighScore::SortScores() {
    std::sort(scores.begin(), scores.end(),
        [](const ScoreEntry& a, const ScoreEntry& b) {
            return a.score > b.score; // Descending order
        });
}