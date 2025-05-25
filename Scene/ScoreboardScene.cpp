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
#include <iostream>

std::vector<Engine::Label*> scoreLabels;  // 追蹤分數標籤
Engine::Label* pageLabel = nullptr;  // 追蹤頁碼標籤

void ScoreboardScene::Initialize() {
    w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    halfW = w / 2;
    halfH = h / 2;
    rendering_page = 0;

    // 添加標題
    AddNewObject(new Engine::Label("SCOREBOARD: ", "pirulen.ttf", 80, halfW, halfH / 9, 0, 255, 255, 255, 0.5, 0.5));

    std::vector<ScoreEntry> scores;
    
    std::ifstream fin("Resource/scoreboard.txt");
    std::string username;
    int score;
    std::string datetime;

    while (fin >> username >> score) {
        // 讀取剩餘的行作為時間資訊
        std::getline(fin, datetime);
        // 移除開頭的空白
        datetime = datetime.substr(1);
        scores.push_back({username, score, datetime});
    }
    fin.close();

    // 按分數排序
    std::sort(scores.begin(), scores.end(), 
        [](const auto& a, const auto& b) { return a.score > b.score; });

    // 刷新 scoreboard 頁面 idx
    pageLabel = new Engine::Label("Page " + std::to_string(rendering_page + 1) + " / " + std::to_string((scores.size() - 1) / 10 + 1), "pirulen.ttf", 48, halfW, halfH / 8 + 100, 0, 255, 255, 255, 0.5, 0.5);
    AddNewObject(pageLabel);

    // 顯示前10個最高分
    int y = halfH / 3 + 100;
    RenderScoreboard(scores, 0);

    // 添加返回按鈕
    Engine::ImageButton *back_btn;
    Engine::ImageButton *prev_btn;
    Engine::ImageButton *next_btn;
    int icon_gap = 300;
    int btn_width = 210;
    int btn_height = 80;
    int btn_font_size = 36;
    float btn_y_offset = 1.8;

    back_btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 100, halfH * btn_y_offset - 50, btn_width, btn_height);
    prev_btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 1100, halfH * btn_y_offset - 50, btn_width, btn_height);
    next_btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 1350, halfH * btn_y_offset - 50, btn_width, btn_height);
    
    
    back_btn->SetOnClickCallback(std::bind(&ScoreboardScene::BackOnClick, this));
    prev_btn->SetOnClickCallback(std::bind(&ScoreboardScene::RenderScoreboard, this, scores, -1));
    next_btn->SetOnClickCallback(std::bind(&ScoreboardScene::RenderScoreboard, this, scores, 1));


    AddNewControlObject(back_btn);
    AddNewControlObject(prev_btn);
    AddNewControlObject(next_btn);
    
    AddNewObject(new Engine::Label("BACK", "pirulen.ttf", btn_font_size, 200, halfH * btn_y_offset - 15, 0, 0, 0, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("PREV", "pirulen.ttf", btn_font_size, 1200, halfH * btn_y_offset - 15, 0, 0, 0, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("NEXT", "pirulen.ttf", btn_font_size, 1450, halfH * btn_y_offset - 15, 0, 0, 0, 255, 0.5, 0.5));

    // 播放背景音樂
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);
}

void ScoreboardScene::Terminate() {
    std::cout << "ScoreboardScene Terminated" << std::endl;
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    
    // 清理分數標籤
    for(auto label : scoreLabels) {
        RemoveObject(label->GetObjectIterator());
    }
    scoreLabels.clear();
    
    scores.clear();
    
    // 清理頁碼標籤
    if (pageLabel) {
        RemoveObject(pageLabel->GetObjectIterator());
        pageLabel = nullptr;
    }
    IScene::Terminate();
}

void ScoreboardScene::RenderScoreboard(const std::vector<ScoreEntry>& scores, int direction) {
    if (direction == -1 && rendering_page > 0) {
        rendering_page--;
    } else if (direction == 1 && rendering_page < scores.size() / 11) {
        rendering_page++;
    }

    // 更新頁碼標籤
    if (pageLabel) {
        pageLabel->Text = "Page " + std::to_string(rendering_page + 1) + " / " + std::to_string((scores.size() - 1) / 10 + 1);
    }

    // 清除舊的分數標籤
    for(auto label : scoreLabels) {
        RemoveObject(label->GetObjectIterator());
    }
    scoreLabels.clear();  // 清空追蹤列表

    int y = halfH / 3 + 40;
    for (size_t i = rendering_page * 10; i < std::min(scores.size(), size_t(rendering_page * 10 + 10)); i++) {
        // 格式化顯示文字，加入時間資訊
        std::string text = std::to_string(i) + ". " + scores[i].username + ": " + std::to_string(scores[i].score) + " (" + scores[i].datetime + ")";
        Engine::Label *new_score_obj_id = new Engine::Label(text, "pirulen.ttf", 32, 100, y, 255, 255, 255, 255, 0, 0);
        scoreLabels.push_back(new_score_obj_id);
        AddNewObject(new_score_obj_id);
        y += 50;
    }
}

void ScoreboardScene::BackOnClick() {
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
} 