#ifndef TOOLBUTTON_HPP
#define TOOLBUTTON_HPP
#include <string>

#include "Engine/Sprite.hpp"
#include "UI/Component/ImageButton.hpp"

class PlayScene;

class ToolButton : public Engine::ImageButton {
protected:
    PlayScene *getPlayScene();

public:
    int price;
    Engine::Sprite Base;
    Engine::Sprite Tool;
    ToolButton(std::string img, std::string imgIn, Engine::Sprite Base, Engine::Sprite Tool, float x, float y, int price);
    void Update(float deltaTime) override;
    void Draw() const override;
};
#endif   // TOOLBUTTON_HPP 