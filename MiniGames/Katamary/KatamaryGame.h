#pragma once
#include "GameFramework.h"
#include "Camera.h"
#include "OrbitalCameraController.h"

class KatamaryGame : public GameFramework
{
public:
	KatamaryGame(LPCWSTR applicationName);
	void Init(int windowWidth, int windowHeight) override;
	void Update() override;

	OrbitalCameraController* orbitalCamController;
	ParticleSystem* fountainPS;
};
