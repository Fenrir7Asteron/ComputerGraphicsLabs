#pragma once
#include "GameFramework.h"
#include "Camera.h"

class SolarSystemGame : public GameFramework
{
public:
	SolarSystemGame(LPCWSTR applicationName);
	void Init(int windowWidth, int windowHeight) override;
	void FreeGameResources() override;

	Camera* camera;
};

