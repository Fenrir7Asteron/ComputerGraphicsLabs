#include "KatamaryGame.h"
#include "StaticBox.h"
#include "UnlitDiffuseMaterial.h"
#include "PhongCoefficients.h"
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

	PhongCoefficients coeff1 =
	{
		{0.5f, 0.5f, 0.5f, 0.0f},
		{0.99f, 0.94f, 0.81f, 27.90f},
		{0.33f, 0.22f, 0.03f, 0.0f},
	};

	PhongCoefficients coeff2 =
	{
		{0.428f, 0.47f, 0.54f, 0.0f},
		{0.33f, 0.33f, 0.52f, 9.85f},
		{0.11f, 0.06f, 0.11f, 0.0f},
	};

	UnlitDiffuseMaterial* katamaryMat = new UnlitDiffuseMaterial(vertexShaderPath, pixelShaderPath, device, displayWin, L"./Textures/sphere_texture.jpg");
	KatamaryBall* katamary = new KatamaryBall(this, 50.0f, 16, 250.0f, coeff1, Matrix::Identity, katamaryMat, PhysicalLayer::Player);
	AddComponent(katamary);

	UnlitDiffuseMaterial* tableMat = new UnlitDiffuseMaterial(vertexShaderPath, pixelShaderPath, device, displayWin, L"./Textures/wood1.jpg");
	AddComponent(new StaticBox(this, coeff2, Vector3::Down * 100.0f, Quaternion::Identity, {10.0f, 2.0f, 10.0f}, tableMat));

	AddComponent(new Model(this, nullptr, Matrix::CreateScale(Vector3::One * 100.0f) * Matrix::CreateTranslation({ -200.0f, 0.0f, 0.0f }), "../Assets/Models/", "pen.obj", vertexShaderPath, 100.0f, katamaryMat, coeff1));
	AddComponent(new Model(this, nullptr, Matrix::CreateRotationY(XMConvertToRadians(0.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 0.0f }), "../Assets/Models/Banana/", "banana.fbx", vertexShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model(this, nullptr, Matrix::CreateScale(Vector3::One * 0.75f) * Matrix::CreateRotationY(XMConvertToRadians(0.0f)) * Matrix::CreateTranslation({ -200.0f, 0.0f, -200.0f }), "../Assets/Models/Banana/", "banana.fbx", vertexShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model(this, nullptr, Matrix::CreateScale(Vector3::One * 5.0f) * Matrix::CreateTranslation({ 0.0f, 0.0f, -200.0f }), "../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx", vertexShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model(this, nullptr, Matrix::CreateScale(Vector3::One * 7.5f) * Matrix::CreateTranslation({ 100.0f, 0.0f, -300.0f }), "../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx", vertexShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model(this, nullptr, Matrix::CreateScale(Vector3::One * 10.0f) * Matrix::CreateRotationX(XMConvertToRadians(-180.0f)) * Matrix::CreateTranslation({ -150.0f, 0.0f, 200.0f }), "../Assets/Models/Can/", "Can_415g.fbx", vertexShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model(this, nullptr, Matrix::CreateScale(Vector3::One * 12.5f) * Matrix::CreateRotationX(XMConvertToRadians(-180.0f)) * Matrix::CreateTranslation({ -250.0f, 0.0f, -300.0f }), "../Assets/Models/Can/", "Can_415g.fbx", vertexShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model(this, nullptr, Matrix::CreateScale(Vector3::One * 3.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 300.0f }), "../Assets/Models/Miku/", "Miku.obj", vertexShaderPath, 1.0f, katamaryMat, coeff1));

	this->orbitalCamController = new OrbitalCameraController(inputDevice, displayWin, 0.005f, 500.0f, 100.0f, katamary, 1000.0f,
		200.0f,  // minDistanceToBodySurface
		3000.0f // maxDistanceToBodySurface
	);

	cameraControllers.emplace_back(this->orbitalCamController);
	SetCameraController(1);

	Vector4 direction = { 1.0f, -0.5f, 1.0f, 0.0f };
	direction.Normalize();

	dirLight =
	{
		direction,                 // direction
		{1.0f, 1.0f, 1.0f, 1.0f},  // color
		01.5f,                     // diffuse
		00.5f,                     // specular
		01.0f,                     // ambient
	};
}
