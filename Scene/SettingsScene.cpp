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
    AddNewObject(new Engine::Label("SETTINGS", "pirulen.ttf", 120, halfW, halfH / 3 + 50, 0, 255, 255, 255, 0.5, 0.5));

    // --- BGM 滑塊與標籤 ---
    int labelX = halfW - 200;
    int sliderX = labelX + 100;
    int bgmY = halfH - 50;
    AddNewObject(new Engine::Label("BGM:", "pirulen.ttf", 28, labelX, bgmY, 255, 255, 255, 255, 0, 0.5));
    Slider* sliderBGM = new Slider(sliderX, bgmY - 10, 150, 4);
    sliderBGM->SetOnValueChangedCallback(std::bind(&SettingsScene::BGMSlideOnValueChanged, this, std::placeholders::_1));
    AddNewControlObject(sliderBGM);

    // --- SFX 滑塊與標籤 ---
    int sfxY = halfH + 10;
    AddNewObject(new Engine::Label("SFX:", "pirulen.ttf", 28, labelX, sfxY, 255, 255, 255, 255, 0, 0.5));
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
    bgmInstance = AudioHelper::PlaySample("select.ogg", true, bgmVolume);
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
