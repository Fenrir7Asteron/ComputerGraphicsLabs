#include "SolarSystemGame.h"
#include "CelestialBody.h"

SolarSystemGame::SolarSystemGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

void SolarSystemGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);
	
	AddComponent(new CelestialBody(this, 100.0f, { 0.0f, 0.0f, 750.0f }));
}
