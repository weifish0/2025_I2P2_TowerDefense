#ifndef RETREATSCENE_HPP
#define RETREATSCENE_HPP
#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>
#include <memory>

class RetreatScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;

public:
    explicit RetreatScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
};

#endif   // RETREATSCENE_HPP
