// StartGame.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "MyGame.h"

int main()
{
	MyGame game(L"TestGame");
	game.Init(800, 800);
	game.Run();
	return 0;
}