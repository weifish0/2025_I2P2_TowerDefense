#ifndef SCOREBOARDSCENE_HPP
#define SCOREBOARDSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <memory>
#include <vector>
#include <string>
#include <ctime>

#include "Engine/IScene.hpp"

struct ScoreRecord {
    int score;
    std::string datetime;
    int lives;
    int money;
};

class ScoreboardScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::vector<ScoreRecord> scores;
    void LoadScores();
    void SaveScores();
    void DisplayScores();

public:
    explicit ScoreboardScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    static void AddScore(int lives, int money);
};

#endif   // SCOREBOARDSCENE_HPP 