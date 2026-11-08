#define _USE_MATH_DEFINES

#include "SandboxGame.h"
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

SandboxGame::SandboxGame(const std::string& applicationName) : GameFramework::GameFramework(applicationName)
{
}

const std::wstring geometryPassShaderPath = L"./Shaders/GeometryShader.hlsl";
const std::wstring depthShaderPath = L"./Shaders/CascadeGeometryDepthShader.hlsl";

void SandboxGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);
}

void SandboxGame::Update()
{
	GameFramework::Update();
}
