#pragma once
#include "GameFramework.h"

class MyGame : public GameFramework
{
public:
	MyGame(LPCWSTR applicationName);
	void Init(int windowWidth, int windowHeight) override;
};

