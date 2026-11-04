#pragma once
#include "GameFramework.h"

class SandboxGame : public GameFramework
{
public:
	SandboxGame(const std::string& applicationName);
	void Init(int windowWidth, int windowHeight) override;
	void Update() override;
};
