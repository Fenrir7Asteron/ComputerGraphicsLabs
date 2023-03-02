#include "SolarSystemGame.h"
#include "CelestialBody.h"

SolarSystemGame::SolarSystemGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

void SolarSystemGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);

	camera = new Camera(10.0f, 10000.0f, -50.0f, 50.0f, this->screenWidth, this->screenHeight);
	
	AddComponent(new CelestialBody(this, { 0.0f, 0.0f, 2.0f }, 0.3f));
}

void SolarSystemGame::FreeGameResources()
{
	delete camera;
}
