// StartGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define NOMINMAX
#define _USE_MATH_DEFINES

#include <iostream>
#include "KatamaryGame.h"

int main()
{

	KatamaryGame game(L"Katamary Game");
	game.Init(800, 800);
	game.Run();
	return 0;
}