#pragma once
#include <ScoobzEngine/Window.h>
#include <ScoobzEngine/ScoobzEngine.h>

class TombGame{
public:
	TombGame();
	~TombGame();
	//start everything up..
	void Run();

private:
	//FUNCTIONS
	void InitSystems();

	//HANDLES
	ScoobzEngine::ScoobzEngine mEngine;
};

