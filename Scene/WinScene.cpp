#include <functional>
#include <string>
#include <fstream>
#include <chrono>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "WinScene.hpp"

void WinScene::Initialize() {
    ticks = 0;
    is_typing = true;
    username = "";
    
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    
    AddNewObject(new Engine::Image("win/benjamin-sad.png", halfW, halfH, 0, 0, 0.5, 0.5));
    AddNewObject(new Engine::Label("You Win!", "pirulen.ttf", 48, halfW, halfH / 4 - 10, 255, 255, 255, 255, 0.5, 0.5));
    
    // 添加提示文字
    prompt_label = new Engine::Label("Enter your name:", "pirulen.ttf", 36, halfW, halfH / 3, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(prompt_label);
    
    // 添加使用者名稱標籤
    username_label = new Engine::Label("", "pirulen.ttf", 36, halfW, halfH / 3 + 50, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(username_label);
    
    // 添加返回按鈕
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("win/dirt.png", "win/floor.png", halfW - 200, halfH * 7 / 4 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&WinScene::BackOnClick, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));
    
    bgmId = AudioHelper::PlayAudio("win.wav");
}

void WinScene::OnKeyDown(int keyCode) {
    if (!is_typing) return;
    
    if (keyCode == ALLEGRO_KEY_ENTER) {
        // 當按下 Enter 鍵時，保存分數並切換場景
        if (!username.empty()) {
            // 獲取當前時間
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            char buffer[32];
            std::tm* timeinfo = std::localtime(&time_t_now);
            std::strftime(buffer, sizeof(buffer), "%Y.%m.%d %H:%M:%S", timeinfo);
            std::string datetime = buffer;
            
            // 保存分數到檔案
            std::ofstream fout("./Resource/scoreboard.txt", std::ios::app);
            if (fout.is_open()) {
                fout << username << " " << dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetScene("play"))->GetScore()
                     << " " << datetime << "\n";
                fout.close();
            }
            is_typing = false;
            Engine::GameEngine::GetInstance().ChangeScene("stage-select");
        }
    }
    else if (keyCode == ALLEGRO_KEY_BACKSPACE) {
        // 處理退格鍵
        if (!username.empty()) {
            username.pop_back();
            username_label->Text = username;
        }
    }
    else if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
        // 處理字母鍵
        if (username.length() < 10) {  // 限制使用者名稱長度
            username += static_cast<char>('a' + (keyCode - ALLEGRO_KEY_A));
            username_label->Text = username;
        }
    }
}

void WinScene::Terminate() {
    IScene::Terminate();
    AudioHelper::StopBGM(bgmId);
}

void WinScene::Update(float deltaTime) {
    ticks += deltaTime;
    if (ticks > 4 && ticks < 100 &&
        dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetScene("play"))->MapId == 2) {
        ticks = 100;
        bgmId = AudioHelper::PlayBGM("happy.ogg");
    }
}

void WinScene::BackOnClick(int stage) {
    // Change to select scene.
    Engine::GameEngine::GetInstance().ChangeScene("stage-select");
}
