#ifndef TOOL_HPP
#define TOOL_HPP
#include <allegro5/base.h>
#include <string>

#include "Engine/Sprite.hpp"

class PlayScene;

class Tool : public Engine::Sprite {
protected:
    int price;
    Sprite imgBase;
    PlayScene *getPlayScene();

public:
    bool Enabled = true;
    bool Preview = false;
    Tool(std::string imgBase, std::string imgTool, float x, float y, int price);
    void Update(float deltaTime) override;
    void Draw() const override;
    int GetPrice() const;
    // 虛擬函數，用於處理工具的使用效果
    virtual void UseTool() = 0;
};
#endif   // TOOL_HPP 