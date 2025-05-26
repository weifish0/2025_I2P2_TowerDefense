#include <cmath>

#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"
#include "Scene/PlayScene.hpp"
#include "ShovelTool.hpp"
#include "Turret/Turret.hpp"
#include "Enemy/Enemy.hpp"

const int ShovelTool::Price = 0;  // 鏟子工具是免費的

ShovelTool::ShovelTool(float x, float y)
    : Tool("play/tower-base.png", "play/shovel.png", x, y, Price) {
    // 移動中心點向下，因為鏟子圖示稍微偏上
    Anchor.y += 8.0f / GetBitmapHeight();
}

void ShovelTool::UseTool() {
    PlayScene *scene = getPlayScene();
    // 獲取滑鼠位置對應的格子座標
    int x = floor(Position.x / PlayScene::BlockSize);
    int y = floor(Position.y / PlayScene::BlockSize);
    
    // 檢查是否在有效範圍內
    if (x < 0 || x >= PlayScene::MapWidth || y < 0 || y >= PlayScene::MapHeight)
        return;
    
    // 檢查該位置是否有砲塔
    for (auto &it : scene->TowerGroup->GetObjects()) {
        Turret *turret = dynamic_cast<Turret *>(it);
        if (!turret)
            continue;
            
        int turretX = floor(turret->Position.x / PlayScene::BlockSize);
        int turretY = floor(turret->Position.y / PlayScene::BlockSize);
        
        if (turretX == x && turretY == y) {
            // 移除砲塔
            scene->TowerGroup->RemoveObject(turret->GetObjectIterator());
            // 添加爆炸效果
            Engine::Sprite *explosion = new Engine::Sprite("play/explosion-2.png", x * PlayScene::BlockSize + PlayScene::BlockSize / 2, y * PlayScene::BlockSize + PlayScene::BlockSize / 2);
            scene->GroundEffectGroup->AddNewObject(explosion);
            break;
        }
    }
} 