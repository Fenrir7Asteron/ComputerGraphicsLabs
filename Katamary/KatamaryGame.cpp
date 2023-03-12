#include "KatamaryGame.h"
#include "StaticBox.h"
#include "UnlitDiffuseMaterial.h"
#include "Model.h"

using namespace DirectX;
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

	AddComponent(new Model(this, nullptr, Matrix::CreateTranslation({ -200.0f, 0.0f, 0.0f }), "../Assets/Models/pen.obj", 10000.0f, katamaryMat));
	AddComponent(new Model(this, nullptr, Matrix::CreateRotationY(XMConvertToRadians(20.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 0.0f }), "../Assets/Models/Banana/banana.fbx", 1.0f, katamaryMat));
	AddComponent(new Model(this, nullptr, Matrix::CreateTranslation({ 0.0f, 0.0f, -200.0f }), "../Assets/Models/Grass_block.fbx", 5.0f, katamaryMat));
	AddComponent(new Model(this, nullptr, Matrix::CreateRotationX(XMConvertToRadians(-180.0f)) * Matrix::CreateTranslation({ -150.0f, 0.0f, 200.0f }), "../Assets/Models/Can_415g.fbx", 10.0f, katamaryMat));
	AddComponent(new Model(this, nullptr, Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 300.0f }), "../Assets/Models/Miku/Miku.obj", 3.0f, katamaryMat));

	this->orbitalCamController = new OrbitalCameraController(inputDevice, displayWin, 0.005f, 500.0f, 2000.0f, katamary, 1000.0f,
		200.0f,  // minDistanceToBodySurface
		3000.0f // maxDistanceToBodySurface
	);

	cameraControllers.emplace_back(this->orbitalCamController);
	SetCameraController(1);
}
