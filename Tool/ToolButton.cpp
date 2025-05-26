#include <allegro5/color.h>

#include "Engine/GameEngine.hpp"
#include "Engine/IScene.hpp"
#include "Scene/PlayScene.hpp"
#include "ToolButton.hpp"

PlayScene *ToolButton::getPlayScene() {
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}

ToolButton::ToolButton(std::string img, std::string imgIn, Engine::Sprite Base, Engine::Sprite Tool, float x, float y, int price) 
    : ImageButton(img, imgIn, x, y), price(price), Base(Base), Tool(Tool) {
}

void ToolButton::Update(float deltaTime) {
    ImageButton::Update(deltaTime);
    if (getPlayScene()->GetMoney() >= price) {
        Enabled = true;
        Base.Tint = Tool.Tint = al_map_rgba(255, 255, 255, 255);
    } else {
        Enabled = false;
        Base.Tint = Tool.Tint = al_map_rgba(0, 0, 0, 160);
    }
}

void ToolButton::Draw() const {
    ImageButton::Draw();
    Base.Draw();
    Tool.Draw();
} 