// StartGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "PongGame.h"

int main()
{
	PongGame game(L"Pong");
	game.Init(800, 800);
	game.Run();
	return 0;
}