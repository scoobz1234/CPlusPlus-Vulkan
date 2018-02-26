#include "stdafx.h"
#include "TombGame.h"

TombGame::TombGame(){}
TombGame::~TombGame(){}

//public function for other classes to call the InitSystems function
void TombGame::Run() {
	InitSystems();
}
//Initialize the systems, and start the gameloop
void TombGame::InitSystems() {
	mEngine.Initvulkan();
	mEngine.GameLoop();
}

