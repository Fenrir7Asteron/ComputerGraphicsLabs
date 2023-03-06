#include "SolarSystemGame.h"

using namespace DirectX::SimpleMath;

SolarSystemGame::SolarSystemGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

void SolarSystemGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);
	
	CelestialBody* Sun     = new CelestialBody(this, 5000.0f, 8, 0.0f,     Vector3::Forward, 27.0f, 0.0f,         nullptr,     Vector3::Up, Vector3::Up);
	CelestialBody* Mercury = new CelestialBody(this, 500.0f,  8, 10000.0f, Vector3::Forward, 58.6f, 87.97f,           Sun,     Vector3::Up, Vector3::Up);
	CelestialBody* Venus   = new CelestialBody(this, 1000.0f, 8, 20000.0f, Vector3::Forward, 243.0f, 224.7f,          Sun,     Vector3::Up, Vector3::Up);
																		    
	CelestialBody* Earth     = new CelestialBody(this, 1000.0f, 8, 30000.0f, Vector3::Forward, 0.99f, 365.26f,          Sun,     Vector3::Up, Vector3::Up);
	CelestialBody* EarthMoon = new CelestialBody(this, 250.0f, 8, 2000.0f, Vector3::Right, 0.99f, 0.99f,              Earth, Vector3::Up, Vector3::Forward);
																		    
	CelestialBody* Mars    = new CelestialBody(this, 600.0f,  8, 40000.0f, Vector3::Forward, 1.03f, 1.88f * 365.0f,   Sun,     Vector3::Up, Vector3::Up);
	CelestialBody* Jupiter = new CelestialBody(this, 2000.0f, 8, 50000.0f, Vector3::Forward, 0.41f, 11.86f * 365.0f,  Sun,     Vector3::Up, Vector3::Up);
	CelestialBody* Saturn  = new CelestialBody(this, 1900.0f, 8, 60000.0f, Vector3::Forward, 0.45f, 29.46f * 365.0f,  Sun,     Vector3::Up, Vector3::Up);
	CelestialBody* Uranus  = new CelestialBody(this, 1500.0f, 8, 70000.0f, Vector3::Forward, 0.72f, 84.01f * 365.0f,  Sun,     Vector3::Up, Vector3::Up);
	CelestialBody* Neptune = new CelestialBody(this, 1200.0f, 8, 80000.0f, Vector3::Forward, 0.67f, 164.80f * 365.0f, Sun,     Vector3::Up, Vector3::Up);

	AddComponent(Sun);
	AddComponent(Mercury);
	AddComponent(Venus);
	AddComponent(Earth);
	AddComponent(EarthMoon);
	AddComponent(Mars);
	AddComponent(Jupiter);
	AddComponent(Saturn);
	AddComponent(Uranus);
	AddComponent(Neptune);

	celestialBodies.push_back(Sun);
	celestialBodies.push_back(Mercury);
	celestialBodies.push_back(Venus);
	celestialBodies.push_back(Earth);
	celestialBodies.push_back(EarthMoon);
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
		SetCameraController(1);
		orbitalCamController->SetTargetBody(celestialBodies[9]);
	}

	if (inputDevice->IsKeyDown(Keys::OemMinus))
	{
		SetCameraController(0);
		orbitalCamController->SetTargetBody(nullptr);
	}
}
