#include "SolarSystemGame.h"

SolarSystemGame::SolarSystemGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

void SolarSystemGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);
	
	CelestialBody* Sun     = new CelestialBody(this, 5000.0f, 16, 0.0f, 27.0f, 0.0f);
	CelestialBody* Mercury = new CelestialBody(this, 500.0f,  16, 10000.0f, 58.6f, 87.97f);
	CelestialBody* Venus   = new CelestialBody(this, 1000.0f, 16, 20000.0f, 243.0f, 224.7f);
	CelestialBody* Earth   = new CelestialBody(this, 1000.0f, 16, 30000.0f, 0.99f, 365.26f);
	CelestialBody* Mars    = new CelestialBody(this, 600.0f,  16, 40000.0f, 1.03f, 1.88f * 365.0f);
	CelestialBody* Jupiter = new CelestialBody(this, 2000.0f, 16, 50000.0f, 0.41f, 11.86f * 365.0f);
	CelestialBody* Saturn  = new CelestialBody(this, 1900.0f, 16, 60000.0f, 0.45f, 29.46f * 365.0f);
	CelestialBody* Uranus  = new CelestialBody(this, 1500.0f, 16, 70000.0f, 0.72f, 84.01f * 365.0f);
	CelestialBody* Neptune = new CelestialBody(this, 1200.0f, 16, 80000.0f, 0.67f, 164.80f * 365.0f);

	AddComponent(Sun);
	AddComponent(Mercury);
	AddComponent(Venus);
	AddComponent(Earth);
	AddComponent(Mars);
	AddComponent(Jupiter);
	AddComponent(Saturn);
	AddComponent(Uranus);
	AddComponent(Neptune);

	celestialBodies.push_back(Sun);
	celestialBodies.push_back(Mercury);
	celestialBodies.push_back(Venus);
	celestialBodies.push_back(Earth);
	celestialBodies.push_back(Mars);
	celestialBodies.push_back(Jupiter);
	celestialBodies.push_back(Saturn);
	celestialBodies.push_back(Uranus);
	celestialBodies.push_back(Neptune);

	this->orbitalCamController = new OrbitalCameraController(inputDevice, displayWin, 0.005f, 500.0f, 10000.0f, Sun, 10000.0f, 100000.0f);
	cameraControllers.emplace_back(this->orbitalCamController);
}

void SolarSystemGame::Update()
{
	GameFramework::Update();

	if (inputDevice->IsKeyDown(Keys::D1))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[0]);
	}

	if (inputDevice->IsKeyDown(Keys::D2))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[1]);
	}

	if (inputDevice->IsKeyDown(Keys::D3))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[2]);
	}

	if (inputDevice->IsKeyDown(Keys::D4))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[3]);
	}

	if (inputDevice->IsKeyDown(Keys::D5))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[4]);
	}

	if (inputDevice->IsKeyDown(Keys::D6))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[5]);
	}

	if (inputDevice->IsKeyDown(Keys::D7))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[6]);
	}

	if (inputDevice->IsKeyDown(Keys::D8))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[7]);
	}

	if (inputDevice->IsKeyDown(Keys::D9))
	{
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[8]);
	}

	if (inputDevice->IsKeyDown(Keys::D0))
	{
		SetCameraController(0);
		orbitalCamController->SetTargetBody(nullptr);
	}
}
