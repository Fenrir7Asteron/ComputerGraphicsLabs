#include "PongGame.h"
#include "PongRacket.h"
#include "PongBall.h"

PongGame::PongGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

void PongGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);

	AddComponent(new PongRacket(this, { -0.9f, 0.0f, 0.0f }, 0.025f, 0.15f, 1.5f));
	AddComponent(new PongBall(this, { 0.0f, 0.0f, 0.0f }, 0.03f, 0.5f, 1.1f));
}
