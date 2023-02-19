#pragma once
#include "GameFramework.h"

class PongGame : public GameFramework
{
public:
	PongGame(LPCWSTR applicationName);
	void Init(int windowWidth, int windowHeight) override;
};

