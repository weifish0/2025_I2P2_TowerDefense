// [main.cpp]
// This is the entry point of your game.
// You can register your scenes here, and start the game.
#include "Engine/GameEngine.hpp"
#include "Engine/LOG.hpp"
#include "Scene/LoseScene.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/StageSelectScene.hpp"
#include "Scene/WinScene.hpp"
#include "Scene/StartScene.h"
#include "Scene/SettingsScene.hpp"
#include "Scene/ScoreboardScene.hpp"
#include "Scene/RetreatScene.hpp"


int main(int argc, char **argv) {
	Engine::LOG::SetConfig(true);
	Engine::GameEngine& game = Engine::GameEngine::GetInstance();


    game.AddNewScene("stage-select", new StageSelectScene());
	game.AddNewScene("play", new PlayScene());
	game.AddNewScene("lose", new LoseScene());
	game.AddNewScene("win", new WinScene());
	game.AddNewScene("retreat", new RetreatScene());
    game.AddNewScene("start", new StartScene());
    game.AddNewScene("settings", new SettingsScene());
    game.AddNewScene("scoreboard-scene", new ScoreboardScene());

	game.Start("start", 60, 1600, 832);
	
	return 0;
}
