#pragma once
#include <vector>
#include <string>

struct ScoreEntry {
    std::string player_name;
    int score;
    int wave;
};

class HighScore {
public:
    HighScore();
    ~HighScore();

    // Load scores from file
    bool LoadScores();

    // Save scores to file
    bool SaveScores();

    // Add a new score
    void AddScore(const std::string& player_name, int score, int wave);

    // Get top scores (limited to max_scores)
    const std::vector<ScoreEntry>& GetTopScores() const;

    // Check if score qualifies for high score list
    bool IsHighScore(int score) const;

private:
    static constexpr int MAX_SCORES = 10;
    std::vector<ScoreEntry> scores;
    std::string filename;

    // Sort scores in descending order
    void SortScores();
};