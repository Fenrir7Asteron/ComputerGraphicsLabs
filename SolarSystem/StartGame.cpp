// StartGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "SolarSystemGame.h"

int main()
{
	SolarSystemGame game(L"Solar System");
	game.Init(800, 800);
	game.Run();
	return 0;
}