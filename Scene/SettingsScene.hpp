#ifndef SettingsScene_HPP
#define SettingsScene_HPP
#include <memory>
#include <vector>
#include <string>

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>

namespace Engine {
    class Label;
}

class SettingsScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    float bgmVolume;
    float sfxVolume;
    std::vector<std::string> bgmList;
    int currentBGMIndex;
    Engine::Label* currentBGMLabel;

public:
    explicit SettingsScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void BGMSlideOnValueChanged(float value);
    void SFXSlideOnValueChanged(float value);
    void NextBGM();
    void PrevBGM();
    void UpdateBGM();
};

#endif   // SettingsScene_HPP
