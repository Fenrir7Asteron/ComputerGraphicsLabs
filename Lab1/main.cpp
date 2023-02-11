#include "Game.h"

int main()
{
	Game game(L"TestGame");
	game.Init(800, 800);
	game.Run();
	return 0;
}