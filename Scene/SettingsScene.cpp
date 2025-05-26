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

    // 標題往上移
    int titleY = 100;
    AddNewObject(new Engine::Label("SETTINGS", "pirulen.ttf", 120, halfW, titleY, 0, 255, 255, 255, 0.5, 0.5));

    // 初始化背景音樂列表
    bgmList = {"select.ogg", " Undertale-OST-071.ogg", "Stardew-Valley-OST-Summer .ogg"};
    currentBGMIndex = 0;

    // UI 元素間距設計
    int spacing = 60;
    int baseY = titleY + 100; // CURRENT BGM 起始位置

    // CURRENT BGM 標題
    AddNewObject(new Engine::Label("CURRENT BGM:", "pirulen.ttf", 32, halfW, baseY, 255, 255, 255, 255, 0.5, 0.5));

    // 顯示目前音樂名稱（在標題下方）
    int musicNameY = baseY + spacing;
    currentBGMLabel = new Engine::Label(bgmList[currentBGMIndex], "pirulen.ttf", 24, halfW, musicNameY, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(currentBGMLabel);

    // --- BGM 選擇按鈕 ---（在音樂名稱下方）
    int btnWidth = 100;
    int btnHeight = 50;
    int btnY = musicNameY + 50;
    // 上一個音樂按鈕
    Engine::ImageButton* prevBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 
        halfW - btnWidth - 10, btnY, btnWidth, btnHeight);
    prevBtn->SetOnClickCallback(std::bind(&SettingsScene::PrevBGM, this));
    AddNewControlObject(prevBtn);
    AddNewObject(new Engine::Label("PREV", "pirulen.ttf", 24, halfW - btnWidth/2 - 10, btnY + btnHeight/2, 0, 0, 0, 255, 0.5, 0.5));
    // 下一個音樂按鈕
    Engine::ImageButton* nextBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 
        halfW + 10, btnY, btnWidth, btnHeight);
    nextBtn->SetOnClickCallback(std::bind(&SettingsScene::NextBGM, this));
    AddNewControlObject(nextBtn);
    AddNewObject(new Engine::Label("NEXT", "pirulen.ttf", 24, halfW + btnWidth/2 + 10, btnY + btnHeight/2, 0, 0, 0, 255, 0.5, 0.5));

    // --- BGM 音量滑塊與標籤 ---
    int labelX = halfW - 200;
    int sliderX = labelX + 300; // 再往右移
    int sliderWidth = 200;
    int sliderHeight = 4;
    int bgmY = btnY + btnHeight + spacing;
    int labelFontSize = 28;
    int sliderOffsetY = 0; // 讓 label 與 slider 垂直置中
    AddNewObject(new Engine::Label("BGM VOLUME:", "pirulen.ttf", labelFontSize, labelX, bgmY + sliderOffsetY, 255, 255, 255, 255, 0, 0.5));
    Slider* sliderBGM = new Slider(sliderX, bgmY - sliderHeight / 2, sliderWidth, sliderHeight);
    sliderBGM->SetOnValueChangedCallback(std::bind(&SettingsScene::BGMSlideOnValueChanged, this, std::placeholders::_1));
    AddNewControlObject(sliderBGM);

    // --- SFX 滑塊與標籤 ---
    int sfxY = bgmY + spacing;
    AddNewObject(new Engine::Label("SFX VOLUME:", "pirulen.ttf", labelFontSize, labelX, sfxY + sliderOffsetY, 255, 255, 255, 255, 0, 0.5));
    Slider* sliderSFX = new Slider(sliderX, sfxY - sliderHeight / 2, sliderWidth, sliderHeight);
    sliderSFX->SetOnValueChangedCallback(std::bind(&SettingsScene::SFXSlideOnValueChanged, this, std::placeholders::_1));
    AddNewControlObject(sliderSFX);

    // 添加返回按鈕
    int backBtnY = sfxY + spacing + 40;
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - 200, backBtnY, 400, 100);
    btn->SetOnClickCallback(std::bind(&SettingsScene::BackOnClick, this, 1));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("BACK", "pirulen.ttf", 48, halfW, backBtnY + 50, 0, 0, 0, 255, 0.5, 0.5));

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
    if (currentBGMLabel) {
        currentBGMLabel->Text = bgmList[currentBGMIndex];
    }
    AudioHelper::CurrentBGMFile = bgmList[currentBGMIndex];
}
