#include "KatamaryGame.h"
#include "PhongCoefficients.h"
#include "KatamaryBall.h"
#include "Model.h"
#include "PointLight.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

KatamaryGame::KatamaryGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{
}

const LPCWSTR geometryPassShaderPath = L"./Shaders/GeometryShader.hlsl";
const LPCWSTR depthShaderPath = L"./Shaders/CascadeGeometryDepthShader.hlsl";

void KatamaryGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);

	dirLight = new DirectionalLight(
		this,
		{1.0f, -0.5f, 1.0f, 1.0f}, // direction
		{1.0f, 1.0f, 1.0f, 1.0f},  // color
		01.5f,                     // diffuse
		00.5f,                     // specular
		01.0f,                     // ambient
		camera
	);



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

	Material* katamaryMat = new Material(geometryPassShaderPath, depthShaderPath, device, displayWin, L"./Textures/sphere_texture.jpg");
	KatamaryBall<BoundingSphere>* katamary = new KatamaryBall<BoundingSphere>(this, 50.0f, 2000.0f, coeff1, "../Assets/Models/", "beach_ball.fbx", geometryPassShaderPath, depthShaderPath, 0.01f, Matrix::Identity, katamaryMat, PhysicalLayer::Player);
	AddComponent(katamary);

	Material* tableMat = new Material(geometryPassShaderPath, depthShaderPath, device, displayWin, L"./Textures/wood1.jpg");

	float radius = 1000.0f;
	float attenuation = 100.0f;
	Vector4 lightPos = { 0.0f, 0.0f, 0.0f, 1.0f };
	Vector4 lightCol = { 1.0f, 0.0f, 0.0f, 1.0f };
	Model<BoundingSphere>* pointLightSphereMesh = new Model<BoundingSphere>(this, nullptr, Matrix::CreateScale(Vector3::One * radius), "../Assets/Models/", "beach_ball.fbx", geometryPassShaderPath, depthShaderPath, 100.0f, katamaryMat, coeff1);
	pointLights.push_back(new PointLight(
		lightPos,
		lightCol,
		radius,
		attenuation,
		1.0f, 1.0f, 1.0f,
		pointLightSphereMesh
	));

	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 100.0f) * Matrix::CreateTranslation({ -200.0f, 0.0f, 0.0f }), "../Assets/Models/", "pen.obj", geometryPassShaderPath, depthShaderPath, 100.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateRotationY(XMConvertToRadians(0.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 0.0f }), "../Assets/Models/Banana/", "banana.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 0.75f) * Matrix::CreateRotationY(XMConvertToRadians(0.0f)) * Matrix::CreateTranslation({ -200.0f, 0.0f, -200.0f }), "../Assets/Models/Banana/", "banana.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 5.0f) * Matrix::CreateTranslation({ 0.0f, 0.0f, -200.0f }), "../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 7.5f) * Matrix::CreateTranslation({ 100.0f, 0.0f, -300.0f }), "../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 10.0f) * Matrix::CreateRotationX(XMConvertToRadians(-180.0f)) * Matrix::CreateTranslation({ -150.0f, 0.0f, 200.0f }), "../Assets/Models/Can/", "Can_415g.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 12.5f) * Matrix::CreateRotationX(XMConvertToRadians(-180.0f)) * Matrix::CreateTranslation({ -250.0f, 0.0f, -300.0f }), "../Assets/Models/Can/", "Can_415g.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 3.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 300.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 3.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 0.0f, 0.0f, 600.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 3.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 0.0f, 0.0f, 1000.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 15.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 0.0f, 0.0f, 1400.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 100.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(180.0f)) * Matrix::CreateTranslation({ 0.0f, 0.0f, 0.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	
	Model<BoundingOrientedBox>* floor = new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale({ 100000.0f, 20.0f, 100000.0f }) * Matrix::CreateTranslation({ 0.0f, -200.0f, 5000.0f }), "../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1);
	floor->enabled = false;
	floor->castShadows = false;
	AddComponent(floor);

	this->orbitalCamController = new OrbitalCameraController(inputDevice, displayWin, 0.005f, 500.0f, 100.0f, katamary, 1000.0f,
		200.0f,  // minDistanceToBodySurface
		3000.0f // maxDistanceToBodySurface
	);

	cameraControllers.emplace_back(this->orbitalCamController);
	SetCameraController(1);

	Vector4 direction = { 1.0f, -0.5f, 1.0f, 0.0f };
	direction.Normalize();
}
