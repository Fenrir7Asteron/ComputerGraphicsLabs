#include "KatamaryGame.h"
#include "StaticBox.h"
#include "UnlitDiffuseMaterial.h"

using namespace DirectX::SimpleMath;

KatamaryGame::KatamaryGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

const LPCWSTR vertexShaderPath = L"./Shaders/KatamaryShader.hlsl";
const LPCWSTR pixelShaderPath = L"./Shaders/KatamaryShader.hlsl";

void KatamaryGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);

	UnlitDiffuseMaterial* katamaryMat = new UnlitDiffuseMaterial(vertexShaderPath, pixelShaderPath, device, displayWin, L"./Textures/sphere_texture.jpg");
	KatamaryBall* katamary = new KatamaryBall(this, 50.0f, 16, Vector3::Zero, Quaternion::Identity, Vector3::One, katamaryMat);
	AddComponent(katamary);

	UnlitDiffuseMaterial* tableMat = new UnlitDiffuseMaterial(vertexShaderPath, pixelShaderPath, device, displayWin, L"./Textures/wood1.jpg");
	AddComponent(new StaticBox(this, Vector3::Down * 100.0f, Quaternion::Identity, {10.0f, 2.0f, 10.0f}, tableMat));

	this->orbitalCamController = new OrbitalCameraController(inputDevice, displayWin, 0.005f, 500.0f, 2000.0f, katamary, 1000.0f,
		200.0f,  // minDistanceToBodySurface
		3000.0f // maxDistanceToBodySurface
	);

	cameraControllers.emplace_back(this->orbitalCamController);
	SetCameraController(1);
}
