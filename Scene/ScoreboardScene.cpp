#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "Scene/ScoreboardScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"

void ScoreboardScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 添加標題
    AddNewObject(new Engine::Label("SCOREBOARD", "pirulen.ttf", 120, halfW, halfH / 3 + 50, 0, 255, 255, 255, 0.5, 0.5));

    // 讀取並顯示分數
    struct ScoreEntry {
        std::string username;
        int score;
    };
    std::vector<ScoreEntry> scores;
    
    std::ifstream fin("Resource/scoreboard.txt");
    std::string username;
    int score;
    while (fin >> username >> score) {
        scores.push_back({username, score});
    }
    fin.close();

    // 按分數排序
    std::sort(scores.begin(), scores.end(), 
        [](const auto& a, const auto& b) { return a.score > b.score; });

    // 顯示前10個最高分
    int y = halfH / 2 + 150;
    for (size_t i = 0; i < std::min(scores.size(), size_t(10)); i++) {
        std::string text = scores[i].username + ": " + std::to_string(scores[i].score);
        AddNewObject(new Engine::Label(text, "pirulen.ttf", 32, halfW, y, 255, 255, 255, 255, 0.5, 0.5));
        y += 50;
    }

    // 添加返回按鈕
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&ScoreboardScene::BackOnClick, this));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("BACK", "pirulen.ttf", 48, halfW, halfH * 3 / 2, 0, 0, 0, 255, 0.5, 0.5));

    // 播放背景音樂
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);
}

void ScoreboardScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}

void ScoreboardScene::BackOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
} 