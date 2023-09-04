#define _USE_MATH_DEFINES

#include "KatamaryGame.h"
#include "PhongCoefficients.h"
#include "KatamaryBall.h"
#include "Model.h"
#include "PointLight.h"
#include "ParticleSystemSorted.h"

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

	PhongCoefficients coeff3 =
	{
		{0.4f, 0.4f, 0.4f, 0.0f},
		{0.0f, 0.0f, 0.0f, 100.0f},
		{0.1f, 0.1f, 0.1f, 0.0f},
	};

	Material* katamaryMat = new Material(geometryPassShaderPath, depthShaderPath, device, displayWin, L"./Textures/sphere_texture.jpg");
	Material* whiteMat = new Material(geometryPassShaderPath, depthShaderPath, device, displayWin, L"./Textures/White.png");
	KatamaryBall<BoundingSphere>* katamary = new KatamaryBall<BoundingSphere>(this, 50.0f, 500.0f, coeff2, "../Assets/Models/", "sphere.fbx", geometryPassShaderPath, depthShaderPath, 0.01f, Matrix::Identity, katamaryMat, PhysicalLayer::Player);
	AddComponent(katamary);

	Material* tableMat = new Material(geometryPassShaderPath, depthShaderPath, device, displayWin, L"./Textures/wood1.jpg");

	int lightRows = 3;
	int lightColumns = 3;
	float verticalRange = 500.0f;
	float horizontalRange = 500.0f;
	float verticalStep = 2.0f * verticalRange / lightRows;
	float horizontalStep = 2.0f *  horizontalRange / lightColumns;
	float radius = 500.0f;
	
	float intensity = 4.0f;
	for (float i = -verticalRange; i < verticalRange; i += verticalStep)
	{
		for (float j = -horizontalRange; j < horizontalRange; j += horizontalStep)
		{
			float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			
			Vector3 pos = { j, radius * 0.5f, i };
			Model<BoundingSphere>* pointLightSphereMesh = new Model<BoundingSphere>(this, nullptr, Matrix::CreateScale(Vector3::One * radius) * Matrix::CreateTranslation(pos), "../Assets/Models/", "sphere.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1);
			pointLights.push_back(new PointLight(
				pos,
				{ r, g, b, 1.0f },
				radius,
				intensity, intensity,
				pointLightSphereMesh
			));
		}
	}

	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 100.0f) * Matrix::CreateTranslation({ -200.0f, 0.0f, 0.0f }), "../Assets/Models/", "pen.obj", geometryPassShaderPath, depthShaderPath, 100.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateRotationY(XMConvertToRadians(0.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 0.0f }), "../Assets/Models/Banana/", "banana.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 0.75f) * Matrix::CreateRotationY(XMConvertToRadians(0.0f)) * Matrix::CreateTranslation({ -200.0f, 0.0f, -200.0f }), "../Assets/Models/Banana/", "banana.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 5.0f) * Matrix::CreateTranslation({ 0.0f, 0.0f, -200.0f }), "../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff2));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 7.5f) * Matrix::CreateTranslation({ 100.0f, 0.0f, -300.0f }), "../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 10.0f) * Matrix::CreateRotationX(XMConvertToRadians(-180.0f)) * Matrix::CreateTranslation({ -150.0f, 0.0f, 200.0f }), "../Assets/Models/Can/", "Can_415g.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff2));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 12.5f) * Matrix::CreateRotationX(XMConvertToRadians(-180.0f)) * Matrix::CreateTranslation({ -250.0f, 0.0f, -300.0f }), "../Assets/Models/Can/", "Can_415g.fbx", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 3.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 200.0f, 0.0f, 0.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 3.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ 0.0f, 0.0f, 600.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff2));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 3.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ -100.0f, 0.0f, 100.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 15.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f)) * Matrix::CreateTranslation({ -150.0f, 0.0f, -200.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff2));
	AddComponent(new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale(Vector3::One * 100.0f) * Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(180.0f)) * Matrix::CreateTranslation({ 0.0f, 0.0f, 0.0f }), "../Assets/Models/Miku/", "Miku.obj", geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1));
	
	Model<BoundingOrientedBox>* floor = new Model<BoundingOrientedBox>(this, nullptr, Matrix::CreateScale({ 100.0f, 1.0f, 100.0f }) * Matrix::CreateTranslation({ -2000.0f, -100.0f, -2000.0f }), "../Assets/Models/", "cube.obj", geometryPassShaderPath, depthShaderPath, 100.0f, whiteMat, coeff3);
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

	fountainPS = new ParticleSystemSorted(this, nullptr, Vector3::Up * 200.0f + Vector3::Backward * 200.0f + Vector3::Right * 500.0f, Quaternion::Identity, Vector3::One, camera);
	AddParticleSystem(fountainPS);
}

void KatamaryGame::Update()
{
	GameFramework::Update();

	if (fountainPS != nullptr)
	{
		for (int i = 0; i < 5; ++i)
		{
			ParticleSystemSorted::Particle p;
	
			Vector3 pos = fountainPS->GetWorldMatrix().Translation();
			p.Position = { pos.x, pos.y, pos.z, 1.0f };

			Vector3 upVector = Vector3::Transform(Vector3::Up, fountainPS->rotation);
			Vector3 rightVector = Vector3::Transform(Vector3::Right, fountainPS->rotation);
			Vector3 forwardVector = Vector3::Transform(Vector3::Forward, fountainPS->rotation);

			Vector3 velocity = upVector * 1000.0f;
			float angle = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * M_PI * 2.0f;
			float offsetPower = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 300.0f;
			velocity += rightVector * sin(angle) * offsetPower;
			velocity += forwardVector * cos(angle) * offsetPower;
			p.Velocity = { velocity.x, velocity.y, velocity.z, 0.0f };

			float g0 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 0.1f;
			float b0 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * 0.1f;
			p.Color0 = { 0.0f, g0, b0, 1.0f };
			p.Color1 = { 1.0f, 0.0f, 0.0f, 1.0f };

			p.Size0Size1 = { 1.0f, 0.1f };

			p.LifeTime = 15.0f;

			fountainPS->AddParticle(p);
		}
	}
}
