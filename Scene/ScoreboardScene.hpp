#ifndef SCOREBOARDSCENE_HPP
#define SCOREBOARDSCENE_HPP
#include <allegro5/allegro_audio.h>
#include <memory>

#include "Engine/IScene.hpp"

class ScoreboardScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;

public:
    // 讀取並顯示分數
    struct ScoreEntry {
        std::string username;
        int score;
    };
    int w;
    int h;
    int halfW;
    int halfH;
    int rendering_page;
    std::vector<ScoreEntry> scores;

    explicit ScoreboardScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick();
    void RenderScoreboard(const std::vector<ScoreEntry>& scores, int direction);
};

#endif   // SCOREBOARDSCENE_HPP 