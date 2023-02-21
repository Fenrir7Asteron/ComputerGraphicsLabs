#include "PongGame.h"
#include "PongRacket.h"
#include "PongBall.h"
#include "PongWall.h"

PongGame::PongGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

void PongGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);

	AddComponent(new PongRacket(this, PhysicalLayer::Player, { -0.9f, 0.0f, 0.0f }, 0.05f, 0.3f, 1.5f, 60.0f));
	AddComponent(new PongRacket(this, PhysicalLayer::Enemy, { 0.9f, 0.0f, 0.0f }, 0.05f, 0.3f, 1.5f, 60.0f));
	AddComponent(new PongWall(this, PhysicalLayer::Default, { 0.0f, 1.0f, 0.0f }, 0.05f, 2.0f, { 0.0f, -1.0f, 0.0f }));
	AddComponent(new PongWall(this, PhysicalLayer::Default, { 0.0f, -1.0f, 0.0f }, 0.05f, 2.0f, { 0.0f, 1.0f, 0.0f }));
	AddComponent(new PongBall(this, { 0.0f, 0.0f, 0.0f }, 0.03f, 0.5f, 1.1f));
}
