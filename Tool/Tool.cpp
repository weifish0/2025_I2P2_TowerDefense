#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>

#include "Engine/GameEngine.hpp"
#include "Engine/IScene.hpp"
#include "Scene/PlayScene.hpp"
#include "Tool.hpp"

PlayScene *Tool::getPlayScene() {
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}

Tool::Tool(std::string imgBase, std::string imgTool, float x, float y, int price) 
    : Sprite(imgTool, x, y), price(price), imgBase(imgBase, x, y) {
}

void Tool::Update(float deltaTime) {
    Sprite::Update(deltaTime);
    PlayScene *scene = getPlayScene();
    imgBase.Position = Position;
    imgBase.Tint = Tint;
    
    if (!Enabled)
        return;
}

void Tool::Draw() const {
    if (Preview) {
        // 在預覽模式下顯示半透明的綠色圓圈
        al_draw_filled_circle(Position.x, Position.y, 32, al_map_rgba(0, 255, 0, 50));
    }
    imgBase.Draw();
    Sprite::Draw();
    if (PlayScene::DebugMode) {
        // 在除錯模式下顯示工具範圍
        al_draw_circle(Position.x, Position.y, 32, al_map_rgb(0, 0, 255), 2);
    }
}

int Tool::GetPrice() const {
    return price;
} 