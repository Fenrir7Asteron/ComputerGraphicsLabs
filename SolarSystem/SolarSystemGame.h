#pragma once
#include "GameFramework.h"
#include "Camera.h"
#include "CelestialBody.h"
#include "OrbitalCameraController.h"

class SolarSystemGame : public GameFramework
{
public:
	SolarSystemGame(LPCWSTR applicationName);
	void Init(int windowWidth, int windowHeight) override;
	virtual void Update() override;

	std::vector<CelestialBody*> celestialBodies;
	OrbitalCameraController* orbitalCamController;
};

