#include <allegro5/allegro_audio.h>
#include <functional>
#include <memory>
#include <string>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/SettingsScene.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"

void SettingsScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 添加標題
    AddNewObject(new Engine::Label("SETTINGS", "pirulen.ttf", 120, halfW / 2, halfH / 3 + 50, 0, 255, 255, 255, 0.5, 0.5));

    // 初始化背景音樂列表
    bgmList = {"select.ogg", "play.ogg", "win.wav"};
    currentBGMIndex = 0;

    // --- BGM 選擇按鈕 ---
    int btnWidth = 100;
    int btnHeight = 50;
    int btnY = halfH - 100;
    
    // 上一個音樂按鈕
    Engine::ImageButton* prevBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 
        halfW - btnWidth - 10, btnY, btnWidth, btnHeight);
    prevBtn->SetOnClickCallback(std::bind(&SettingsScene::PrevBGM, this));
    AddNewControlObject(prevBtn);
    AddNewObject(new Engine::Label("Prev", "pirulen.ttf", 24, halfW - btnWidth/2 - 10, btnY + btnHeight/2, 0, 0, 0, 255, 0.5, 0.5));

    // 下一個音樂按鈕
    Engine::ImageButton* nextBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 
        halfW + 10, btnY, btnWidth, btnHeight);
    nextBtn->SetOnClickCallback(std::bind(&SettingsScene::NextBGM, this));
    AddNewControlObject(nextBtn);
    AddNewObject(new Engine::Label("Next", "pirulen.ttf", 24, halfW + btnWidth/2 + 10, btnY + btnHeight/2, 0, 0, 0, 255, 0.5, 0.5));

    // 當前音樂名稱標籤
    AddNewObject(new Engine::Label("Current BGM:", "pirulen.ttf", 28, halfW, btnY - 30, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label(bgmList[currentBGMIndex], "pirulen.ttf", 24, halfW, btnY + btnHeight + 30, 255, 255, 255, 255, 0.5, 0.5));

    // --- BGM 音量滑塊與標籤 ---
    int labelX = halfW - 200;
    int sliderX = labelX + 100;
    int bgmY = halfH;
    AddNewObject(new Engine::Label("BGM Volume:", "pirulen.ttf", 28, labelX, bgmY, 255, 255, 255, 255, 0, 0.5));
    Slider* sliderBGM = new Slider(sliderX, bgmY - 10, 150, 4);
    sliderBGM->SetOnValueChangedCallback(std::bind(&SettingsScene::BGMSlideOnValueChanged, this, std::placeholders::_1));
    AddNewControlObject(sliderBGM);

    // --- SFX 滑塊與標籤 ---
    int sfxY = halfH + 60;
    AddNewObject(new Engine::Label("SFX Volume:", "pirulen.ttf", 28, labelX, sfxY, 255, 255, 255, 255, 0, 0.5));
    Slider* sliderSFX = new Slider(sliderX, sfxY - 10, 150, 4);
    sliderSFX->SetOnValueChangedCallback(std::bind(&SettingsScene::SFXSlideOnValueChanged, this, std::placeholders::_1));
    AddNewControlObject(sliderSFX);

    // 添加返回按鈕
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, halfH * 3 / 2 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&SettingsScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("BACK", "pirulen.ttf", 48, halfW, halfH * 3 / 2, 0, 0, 0, 255, 0.5, 0.5));

    // 初始化音量值
    bgmVolume = AudioHelper::BGMVolume;
    sfxVolume = AudioHelper::SFXVolume;
    
    // 播放背景音樂
    UpdateBGM();
    sliderBGM->SetValue(bgmVolume);
    sliderSFX->SetValue(sfxVolume);
}

void SettingsScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}

void SettingsScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}

void SettingsScene::BGMSlideOnValueChanged(float value) {
    bgmVolume = value;
    AudioHelper::ChangeSampleVolume(bgmInstance, value);
    AudioHelper::BGMVolume = value;
}

void SettingsScene::SFXSlideOnValueChanged(float value) {
    sfxVolume = value;
    AudioHelper::SFXVolume = value;
}

void SettingsScene::NextBGM() {
    currentBGMIndex = (currentBGMIndex + 1) % bgmList.size();
    UpdateBGM();
}

void SettingsScene::PrevBGM() {
    currentBGMIndex = (currentBGMIndex - 1 + bgmList.size()) % bgmList.size();
    UpdateBGM();
}

void SettingsScene::UpdateBGM() {
    if (bgmInstance) {
        AudioHelper::StopSample(bgmInstance);
    }
    bgmInstance = AudioHelper::PlaySample(bgmList[currentBGMIndex], true, bgmVolume);
}
