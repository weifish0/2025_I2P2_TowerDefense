#include <algorithm>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>
#include <iostream>

#include "Enemy/Enemy.hpp"
#include "Enemy/SoldierEnemy.hpp"
#include "Enemy/TankEnemy.hpp"
#include "Enemy/RedTankEnemy.hpp"
#include "Enemy/PlaneEnemy.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Turret/LaserTurret.hpp"
#include "Turret/MissileTurret.hpp"
#include "Turret/MachineGunTurret.hpp"
#include "Turret/TurretButton.hpp"
#include "Tool/ToolButton.hpp"
#include "Tool/Tool.hpp"
#include "Tool/ShovelTool.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/Plane.hpp"
#include "UI/Component/Label.hpp"


bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);
const std::vector<int> PlayScene::code = {
    ALLEGRO_KEY_UP, ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_DOWN,
    ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT,
    ALLEGRO_KEY_B, ALLEGRO_KEY_A, ALLEGRO_KEY_LSHIFT, ALLEGRO_KEY_ENTER
};
Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize() {
    mapState.clear();
    keyStrokes.clear();
    ticks = 0;
    deathCountDown = -1;
    lives = 10;
    money = 150;
    score = 0;
    SpeedMult = 1;
    is_score_saved = false;
    retreatButtonVisible = false;
    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());
    AddNewObject(GroundEffectGroup = new Group());
    AddNewObject(DebugIndicatorGroup = new Group());
    AddNewObject(TowerGroup = new Group());
    AddNewObject(EnemyGroup = new Group());
    AddNewObject(BulletGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    // Should support buttons.
    AddNewControlObject(UIGroup = new Group());
    ReadMap();
    ReadEnemyWave();
    mapDistance = CalculateBFSDistance();
    ConstructUI();
    imgTarget = new Engine::Image("play/target.png", 0, 0);
    imgTarget->Visible = false;
    preview = nullptr;
    UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    // Start BGM.
    bgmId = AudioHelper::PlayBGM(AudioHelper::CurrentBGMFile);
}
void PlayScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    AudioHelper::StopSample(deathBGMInstance);
    deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}

void PlayScene::SaveScore() {
    if (is_score_saved) {
        return;
    }
    
    // 獲取當前時間
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // 格式化時間
    char buffer[32];
    std::tm* timeinfo = std::localtime(&time_t_now);
    std::strftime(buffer, sizeof(buffer), "%Y.%m.%d %H:%M:%S", timeinfo);
    std::string datetime = buffer;
    
    // 保存分數到檔案
    std::ofstream fout("./Resource/scoreboard.txt", std::ios::app);
    if (fout.is_open()) {
        fout << "Unknown" << " " << score << " " << datetime << "\n";
        fout.close();
        Engine::LOG(Engine::INFO) << "分數已成功寫入檔案";
    } else {
        Engine::LOG(Engine::ERROR) << "無法開啟分數檔案進行寫入";
    }
    is_score_saved = true;
}

void PlayScene::Update(float deltaTime) {
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
    if (SpeedMult == 0)
        deathCountDown = -1;
    else if (deathCountDown != -1)
        SpeedMult = 1;
    // Calculate danger zone.
    std::vector<float> reachEndTimes;
    for (auto &it : EnemyGroup->GetObjects()) {
        reachEndTimes.push_back(dynamic_cast<Enemy *>(it)->reachEndTime);
    }
    // Can use Heap / Priority-Queue instead. But since we won't have too many enemies, sorting is fast enough.
    std::sort(reachEndTimes.begin(), reachEndTimes.end());
    float newDeathCountDown = -1;
    int danger = lives;
    for (auto &it : reachEndTimes) {
        if (it <= DangerTime) {
            danger--;
            if (danger <= 0) {
                // Death Countdown
                float pos = DangerTime - it;
                if (it > deathCountDown) {
                    // Restart Death Count Down BGM.
                    AudioHelper::StopSample(deathBGMInstance);
                    if (SpeedMult != 0)
                        deathBGMInstance = AudioHelper::PlaySample("astronomia.ogg", false, AudioHelper::BGMVolume, pos);
                }
                float alpha = pos / DangerTime;
                alpha = std::max(0, std::min(255, static_cast<int>(alpha * alpha * 255)));
                dangerIndicator->Tint = al_map_rgba(255, 255, 255, alpha);
                newDeathCountDown = it;
                break;
            }
        }
    }
    deathCountDown = newDeathCountDown;
    if (SpeedMult == 0)
        AudioHelper::StopSample(deathBGMInstance);
    if (deathCountDown == -1 && lives > 0) {
        AudioHelper::StopSample(deathBGMInstance);
        dangerIndicator->Tint.a = 0;
    }
    if (SpeedMult == 0)
        deathCountDown = -1;
    for (int i = 0; i < SpeedMult; i++) {
        IScene::Update(deltaTime);
        // Check if we should create new enemy.
        ticks += deltaTime;
        if (enemyWaveData.empty()) {
            if (EnemyGroup->GetObjects().empty()) {
                SaveScore();
                // Win.
                Engine::GameEngine::GetInstance().ChangeScene("win");
            }
            continue;
        }
        auto current = enemyWaveData.front();
        if (ticks < current.second)
            continue;
        ticks -= current.second;
        enemyWaveData.pop_front();
        const Engine::Point SpawnCoordinate = Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2, SpawnGridPoint.y * BlockSize + BlockSize / 2);
        Enemy *enemy;
        switch (current.first) {
            case 1:
                EnemyGroup->AddNewObject(enemy = new SoldierEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 2:
                EnemyGroup->AddNewObject(enemy = new PlaneEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 3:
                EnemyGroup->AddNewObject(enemy = new TankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 4:
                EnemyGroup->AddNewObject(enemy = new RedTankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            default:
                continue;
        }
        enemy->UpdatePath(mapDistance);
        // Compensate the time lost.
        enemy->Update(ticks);
    }
    if (preview) {
        preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        // To keep responding when paused.
        preview->Update(deltaTime);
    }
    if (toolPreview) {
        toolPreview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        // To keep responding when paused.
        toolPreview->Update(deltaTime);
    }
}

void PlayScene::Draw() const {
    IScene::Draw();
    if (DebugMode) {
        // Draw reverse BFS distance on all reachable blocks.
        for (int i = 0; i < MapHeight; i++) {
            for (int j = 0; j < MapWidth; j++) {
                if (mapDistance[i][j] != -1) {
                    // Not elegant nor efficient, but it's quite enough for debugging.
                    Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
                    label.Anchor = Engine::Point(0.5, 0.5);
                    label.Draw();
                }
            }
        }
    }
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
    if ((button & 1) && !imgTarget->Visible) {
        // 取消砲塔或工具的放置
        if (preview) {
            UIGroup->RemoveObject(preview->GetObjectIterator());
            preview = nullptr;
        }
        if (toolPreview) {
            UIGroup->RemoveObject(toolPreview->GetObjectIterator());
            toolPreview = nullptr;
        }
    }
    IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    if ((!preview && !toolPreview) || x < 0 || x >= MapWidth || y < 0 || y >= MapHeight) {
        imgTarget->Visible = false;
        return;
    }
    imgTarget->Visible = true;
    imgTarget->Position.x = x * BlockSize;
    imgTarget->Position.y = y * BlockSize;
}
void PlayScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
    if (!imgTarget->Visible)
        return;
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    if (button & 1) {
        if (preview) {  // 處理砲塔放置
            std::cout << "Trying to place turret at: " << x << ", " << y << std::endl;
            std::cout << "Current map state: " << (mapState[y][x] == TILE_OCCUPIED ? "OCCUPIED" : "NOT OCCUPIED") << std::endl;
            
            if (mapState[y][x] != TILE_OCCUPIED) {
                if (!CheckSpaceValid(x, y)) {
                    Engine::Sprite *sprite;
                    GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
                    sprite->Rotation = 0;
                    return;
                }
                // Purchase.
                EarnMoney(-preview->GetPrice());
                // Remove Preview.
                preview->GetObjectIterator()->first = false;
                UIGroup->RemoveObject(preview->GetObjectIterator());
                // Construct real turret.
                preview->Position.x = x * BlockSize + BlockSize / 2;
                preview->Position.y = y * BlockSize + BlockSize / 2;
                preview->Enabled = true;
                preview->Preview = false;
                preview->Tint = al_map_rgba(255, 255, 255, 255);
                TowerGroup->AddNewObject(preview);
                // To keep responding when paused.
                preview->Update(0);
                // Remove Preview.
                preview = nullptr;

                mapState[y][x] = TILE_OCCUPIED;
                OnMouseMove(mx, my);
            }
        } else if (toolPreview) {  // 處理工具使用
            std::cout << "Using shovel at: " << x << ", " << y << std::endl;
            std::cout << "Current map state: " << (mapState[y][x] == TILE_OCCUPIED ? "OCCUPIED" : "NOT OCCUPIED") << std::endl;
            
            // 使用工具
            toolPreview->Position.x = x * BlockSize + BlockSize / 2;
            toolPreview->Position.y = y * BlockSize + BlockSize / 2;
            toolPreview->UseTool();
            // 移除工具預覽
            UIGroup->RemoveObject(toolPreview->GetObjectIterator());
            toolPreview = nullptr;
        }
    }
}
void PlayScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (keyCode == ALLEGRO_KEY_TAB) {
        DebugMode = !DebugMode;
    } else {
        keyStrokes.push_back(keyCode);
        if (keyStrokes.size() > code.size())
            keyStrokes.pop_front();
    }
    // Debug: 輸出目前keyStrokes內容
    std::cout << "Current keyStrokes: ";
    for (int k : keyStrokes) std::cout << k << ' ';
    std::cout << std::endl;
    if (keyCode == ALLEGRO_KEY_Q) {
        // Hotkey for MachineGunTurret.
        UIBtnClicked(0);
    } else if (keyCode == ALLEGRO_KEY_W) {
        // Hotkey for LaserTurret.
        UIBtnClicked(1);
    }
    else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
        // Hotkey for Speed up.
        SpeedMult = keyCode - ALLEGRO_KEY_0;
    }
    if (keyStrokes.size() == code.size() && std::equal(keyStrokes.begin(), keyStrokes.end(), code.begin())) {
        std::cout << "CHEAT CODE ACTIVATED!" << std::endl;
        EffectGroup->AddNewObject(new Plane());
        EarnMoney(10000);
    }
}
void PlayScene::Hit() {
    lives--;
    score -= 500;
    if (UILives)
        UILives->Text = std::string("Life ") + std::to_string(lives);
    if (lives <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
}
int PlayScene::GetMoney() const {
    return money;
}
void PlayScene::EarnMoney(int money) {
    this->money += money;
    UIMoney->Text = std::string("$") + std::to_string(this->money);
}
void PlayScene::EarnScore(int score) {
    this->score += score;
    UIScore->Text = std::string("Score ") + std::to_string(this->score);
    
    // 檢查分數是否達到 1000
    if (this->score >= 1000 && !retreatButtonVisible) {
        retreatButtonVisible = true;
        if (retreatButton) {
            retreatButton->Visible = true;
            // 找到並顯示標籤
            for (auto &it : UIGroup->GetObjects()) {
                if (auto label = dynamic_cast<Engine::Label *>(it)) {
                    if (label->Text == "Retreat") {
                        label->Visible = true;
                        break;
                    }
                }
            }
        }
    }
}
void PlayScene::ReadMap() {
    std::string filename = std::string("Resource/map") + std::to_string(MapId) + ".txt";
    // Read map file.
    char c;
    std::vector<bool> mapData;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(false); break;
            case '1': mapData.push_back(true); break;
            case '\n':
            case '\r':
                if (static_cast<int>(mapData.size()) / MapWidth != 0)
                    throw std::ios_base::failure("Map data is corrupted.");
                break;
            default: throw std::ios_base::failure("Map data is corrupted.");
        }
    }
    fin.close();
    // Validate map data.
    if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
        throw std::ios_base::failure("Map data is corrupted.");
    // Store map in 2d array.
    mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            const int num = mapData[i * MapWidth + j];
            mapState[i][j] = num ? TILE_FLOOR : TILE_DIRT;
            if (num)
                TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
            else
                TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
        }
    }
}
void PlayScene::ReadEnemyWave() {
    std::string filename = std::string("Resource/enemy") + std::to_string(MapId) + ".txt";
    // Read enemy file.
    float type, wait, repeat;
    enemyWaveData.clear();
    std::ifstream fin(filename);
    while (fin >> type && fin >> wait && fin >> repeat) {
        for (int i = 0; i < repeat; i++)
            enemyWaveData.emplace_back(type, wait);
    }
    fin.close();
}
void PlayScene::ConstructUI() {
    // Background
    UIGroup->AddNewObject(new Engine::Image("play/sand.png", 1280, 0, 320, 832));
    // Text
    UIGroup->AddNewObject(new Engine::Label(std::string("Stage ") + std::to_string(MapId), "pirulen.ttf", 32, 1294, 0));
    UIGroup->AddNewObject(UIMoney = new Engine::Label(std::string("$") + std::to_string(money), "pirulen.ttf", 24, 1294, 48));
    UIGroup->AddNewObject(UILives = new Engine::Label(std::string("Life ") + std::to_string(lives), "pirulen.ttf", 24, 1294, 88));
    UIGroup->AddNewObject(UIScore = new Engine::Label(std::string("Score ") + std::to_string(score), "pirulen.ttf", 24, 1294, 128));
    TurretButton *btn;
    // Button 1
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1294, 188, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-1.png", 1294, 188 - 8, 0, 0, 0, 0), 1294, 188, MachineGunTurret::Price);
    // Reference: Class Member Function Pointer and std::bind.
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 0));
    UIGroup->AddNewControlObject(btn);
    // Button 2
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1370, 188, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-2.png", 1370, 188 - 8, 0, 0, 0, 0), 1370, 188, LaserTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 1));
    UIGroup->AddNewControlObject(btn);
    // Button 3
    btn = new TurretButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1446, 188, 0, 0, 0, 0),
                           Engine::Sprite("play/turret-3.png", 1446, 188 - 8, 0, 0, 0, 0), 1446, 188, MissileTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 2));
    UIGroup->AddNewControlObject(btn);
    // Button 4 - Shovel Tool
    ToolButton *toolBtn = new ToolButton("play/floor.png", "play/dirt.png",
                           Engine::Sprite("play/tower-base.png", 1522, 188, 0, 0, 0, 0),
                           Engine::Sprite("play/shovel.png", 1522, 188 - 8, 0, 0, 0, 0), 1522, 188, 0);
    toolBtn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 3));
    UIGroup->AddNewControlObject(toolBtn);

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int shift = 135 + 25;
    dangerIndicator = new Engine::Sprite("play/benjamin.png", w - shift, h - shift);
    dangerIndicator->Tint.a = 0;
    UIGroup->AddNewObject(dangerIndicator);

    // Retreat Button 右下角
    int retreatBtnX = w - 300;
    int retreatBtnY = h - 200;
    retreatButton = new Engine::ImageButton("play/floor.png", "play/dirt.png", retreatBtnX, retreatBtnY);
    retreatButton->SetOnClickCallback(std::bind(&PlayScene::OnRetreatButtonClicked, this));
    retreatButton->Visible = false;
    UIGroup->AddNewControlObject(retreatButton);
    
    // Retreat Button Label 右下角
    Engine::Label *retreatLabel = new Engine::Label("Retreat", "pirulen.ttf", 24, retreatBtnX + 60, retreatBtnY + 20);
    retreatLabel->Anchor = Engine::Point(0.5, 0.5);
    retreatLabel->Visible = false;
    UIGroup->AddNewObject(retreatLabel);
}

void PlayScene::UIBtnClicked(int id) {
    if (id < 3) {  // 處理砲塔按鈕
        std::cout << "UIBtnClicked: " << id << std::endl;
        // 清除工具預覽
        if (toolPreview) {
            UIGroup->RemoveObject(toolPreview->GetObjectIterator());
            toolPreview = nullptr;
        }

        Turret *next_preview = nullptr;
        if (id == 0 && money >= MachineGunTurret::Price)
            next_preview = new MachineGunTurret(0, 0);
        else if (id == 1 && money >= LaserTurret::Price)
            next_preview = new LaserTurret(0, 0);
        else if (id == 2 && money >= MissileTurret::Price)
            next_preview = new MissileTurret(0, 0);
        
        if (!next_preview)
            return;   // not enough money or invalid turret.

        if (preview)
            UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = next_preview;
        preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        preview->Tint = al_map_rgba(255, 255, 255, 200);
        preview->Enabled = false;
        preview->Preview = true;
        UIGroup->AddNewObject(preview);
    } else if (id == 3) {  // 處理鏟子工具按鈕
        // 清除砲塔預覽
        if (preview) {
            UIGroup->RemoveObject(preview->GetObjectIterator());
            preview = nullptr;
        }

        Tool *next_tool_preview = nullptr;
        next_tool_preview = new ShovelTool(0, 0);
        
        if (!next_tool_preview)
            return;

        if (toolPreview)
            UIGroup->RemoveObject(toolPreview->GetObjectIterator());
        toolPreview = next_tool_preview;
        toolPreview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        toolPreview->Tint = al_map_rgba(255, 255, 255, 200);
        toolPreview->Enabled = false;
        toolPreview->Preview = true;
        UIGroup->AddNewObject(toolPreview);
    }
    OnMouseMove(Engine::GameEngine::GetInstance().GetMousePosition().x, Engine::GameEngine::GetInstance().GetMousePosition().y);
}

void PlayScene::OnRetreatButtonClicked() {
    SaveScore();
    Engine::GameEngine::GetInstance().ChangeScene("retreat");
}

bool PlayScene::CheckSpaceValid(int x, int y) {
    if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
        return false;
    auto map00 = mapState[y][x];
    mapState[y][x] = TILE_OCCUPIED;
    std::vector<std::vector<int>> map = CalculateBFSDistance();
    mapState[y][x] = map00;
    if (map[0][0] == -1)
        return false;
    for (auto &it : EnemyGroup->GetObjects()) {
        Engine::Point pnt;
        pnt.x = floor(it->Position.x / BlockSize);
        pnt.y = floor(it->Position.y / BlockSize);
        if (pnt.x < 0) pnt.x = 0;
        if (pnt.x >= MapWidth) pnt.x = MapWidth - 1;
        if (pnt.y < 0) pnt.y = 0;
        if (pnt.y >= MapHeight) pnt.y = MapHeight - 1;
        if (map[pnt.y][pnt.x] == -1)
            return false;
    }
    // All enemy have path to exit.
    mapState[y][x] = TILE_OCCUPIED;
    mapDistance = map;
    for (auto &it : EnemyGroup->GetObjects())
        dynamic_cast<Enemy *>(it)->UpdatePath(mapDistance);
    return true;
}
std::vector<std::vector<int>> PlayScene::CalculateBFSDistance() {
    // Reverse BFS to find path.
    std::vector<std::vector<int>> map(MapHeight, std::vector<int>(std::vector<int>(MapWidth, -1)));
    std::queue<Engine::Point> que;
    // Push end point.
    // BFS from end point.
    if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
        return map;
    que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
    map[MapHeight - 1][MapWidth - 1] = 0;
    while (!que.empty()) {
        Engine::Point p = que.front();
        que.pop();
        // PROJECT-1 (1/1): Implement a BFS starting from the most right-bottom block in the map.
        //               For each step you should assign the corresponding distance to the most right-bottom block.
        //               mapState[y][x] is TILE_DIRT if it is empty.
        // 檢查四個方向
        for (auto &dir : directions) {
            int x = p.x + dir.x;
            int y = p.y + dir.y;
            // 檢查是否在邊界內且是可行走的地板
            if (x >= 0 && x < MapWidth && y >= 0 && y < MapHeight && 
                mapState[y][x] == TILE_DIRT && map[y][x] == -1) {
                map[y][x] = map[p.y][p.x] + 1;
                que.push(Engine::Point(x, y));
            }
        }
    }
    return map;
}
