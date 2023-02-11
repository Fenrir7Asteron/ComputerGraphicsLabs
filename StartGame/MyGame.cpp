#include "MyGame.h"
#include "AwesomeGameComponent.h"

MyGame::MyGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

void MyGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);

	gameComponents.push_back(new AwesomeGameComponent(this, { 1.0f, -0.5f, 0.0f }));
	gameComponents.push_back(new AwesomeGameComponent(this, { -1.0f, -0.5f, 0.0f }));
	gameComponents.push_back(new AwesomeGameComponent(this, { 0.0f, 0.5f, 0.0f }));
	gameComponents.push_back(new AwesomeGameComponent(this, { -2.0f, 0.5f, 0.0f }));
}
