#define _USE_MATH_DEFINES

#include "KatamaryGame.h"
#include "PhongCoefficients.h"
#include "KatamaryBall.h"
#include "Model.h"
#include "Component/GameObject.h"
#include "Component/Render/Light/PointLight.h"
#include "ParticleSystemSorted.h"
#include <iostream>

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

	auto dirLightObject = CreateGameObject("DirectionalLight");	
	dirLight = dirLightObject->AddComponent<DirectionalLight>();
	dirLight->Bind(
		{ 1.0f, -0.5f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }, 
		01.5f,                    
		00.5f,                 
		01.0f,                    
		camera
	);
	dirLight->enabled = true;


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

	auto* katamaryObject = CreateGameObject("Katamary");
	auto* katamary = katamaryObject->AddComponent<KatamaryBall<BoundingSphere>>();
	katamary->Bind(50.0f, 500.0f, coeff2, "../Assets/Models/", "sphere.fbx", geometryPassShaderPath, depthShaderPath, 0.01f, katamaryMat, PhysicalLayer::Player);

	Material* tableMat = new Material(geometryPassShaderPath, depthShaderPath, device, displayWin, L"./Textures/wood1.jpg");

	int lightRows = 3;
	int lightColumns = 3;
	float verticalRange = 500.0f;
	float horizontalRange = 500.0f;
	float verticalStep = 2.0f * verticalRange / lightRows;
	float horizontalStep = 2.0f *  horizontalRange / lightColumns;
	float radius = 500.0f;
	
	float intensity = 4.0f;
	int pointLightCount = 0;

	if (lightRows * lightColumns > 0)
	{
		for (float i = -verticalRange; i < verticalRange; i += verticalStep)
		{
			for (float j = -horizontalRange; j < horizontalRange; j += horizontalStep)
			{
				pointLightCount++;

				float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

				Vector3 pos = { j, radius * 0.5f, i };

				auto pointLightObject = new GameObject("PointLight " + std::to_string(pointLightCount));
				auto* pointLightTransform = pointLightObject->GetTransform();
				pointLightTransform->Move(pos);
				pointLightTransform->scale = Vector3::One * radius;

				Model<BoundingSphere>* pointLightSphereMesh = pointLightObject->AddComponent<Model<BoundingSphere>>();
				pointLightSphereMesh->Bind(
					"../Assets/Models/", "sphere.fbx",
					geometryPassShaderPath, depthShaderPath, 1.0f, katamaryMat, coeff1);
				pointLightSphereMesh->enabled = false;

				PointLight* pointLight = pointLightObject->AddComponent<PointLight>();
				pointLight->Bind(
					{ r, g, b, 1.0f },
					radius,
					intensity, intensity,
					pointLightSphereMesh
				);

				pointLights.push_back(pointLight);
			}
		}
	}


	SpawnPickableItem(
		"Pen",
		"../Assets/Models/", "pen.obj", 
		katamaryMat, coeff1,
		{ -200.0f, 0.0f, 0.0f }, Quaternion::Identity, Vector3::One * 100.0f,
		100.0f);

	SpawnPickableItem(
		"Banana1",
		"../Assets/Models/Banana/", "banana.fbx",
		katamaryMat, coeff1,
		{ 200.0f, 0.0f, 0.0f });

	SpawnPickableItem(
		"Banana2",
		"../Assets/Models/Banana/", "banana.fbx",
		katamaryMat, coeff1,
		{ -200.0f, 0.0f, -200.0f }, 
		Quaternion::CreateFromAxisAngle(Vector3::Up, XMConvertToRadians(-30.0f)),
			Vector3::One * 0.75f);

	SpawnPickableItem(
		"Grass_block1",
		"../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx",
		katamaryMat, coeff2,
		{ 0.0f, 0.0f, -200.0f }, Quaternion::Identity, Vector3::One * 5.0f);

	SpawnPickableItem(
		"Grass_block2",
		"../Assets/Models/Minecraft_Grass_block/Grass_block/", "Grass_block.fbx",
		katamaryMat, coeff1,
		{ 100.0f, 0.0f, -300.0f }, Quaternion::Identity, Vector3::One * 7.5f);

	SpawnPickableItem(
		"Can1",
		"../Assets/Models/Can/", "Can_415g.fbx",
		katamaryMat, coeff2,
		{ -150.0f, 0.0f, 200.0f },
		Quaternion::CreateFromAxisAngle(Vector3::Right, XMConvertToRadians(-180.0f)),
		Vector3::One * 10.0f);

	SpawnPickableItem(
		"Can2",
		"../Assets/Models/Can/", "Can_415g.fbx",
		katamaryMat, coeff1,
		{ -250.0f, 0.0f, -300.0f },
		Quaternion::CreateFromAxisAngle(Vector3::Right, XMConvertToRadians(-180.0f)),
		Vector3::One * 12.5f);

	SpawnPickableItem(
		"Miku1",
		"../Assets/Models/Miku/", "Miku.obj",
		katamaryMat, coeff1,
		{ 200.0f, 0.0f, 0.0f },
		Quaternion::CreateFromRotationMatrix(
			Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f))),
		Vector3::One * 3.0f);

	SpawnPickableItem(
		"Miku2",
		"../Assets/Models/Miku/", "Miku.obj",
		katamaryMat, coeff2,
		{ 0.0f, 0.0f, 600.0f },
		Quaternion::CreateFromRotationMatrix(
			Matrix::CreateRotationX(XMConvertToRadians(-90.0f)) * Matrix::CreateRotationY(XMConvertToRadians(220.0f))),
		Vector3::One * 3.0f);

	SpawnPickableItem(
		"Miku3",
		"../Assets/Models/Miku/", "Miku.obj",
		katamaryMat, coeff1,
		{ -100.0f, 0.0f, 100.0f },
		Quaternion::CreateFromRotationMatrix(
			Matrix::CreateRotationX(XMConvertToRadians(-90.0f))* Matrix::CreateRotationY(XMConvertToRadians(220.0f))),
		Vector3::One * 3.0f);

	SpawnPickableItem(
		"BigMiku",
		"../Assets/Models/Miku/", "Miku.obj",
		katamaryMat, coeff2,
		{ -150.0f, 0.0f, -200.0f },
		Quaternion::CreateFromRotationMatrix(
			Matrix::CreateRotationX(XMConvertToRadians(-90.0f))* Matrix::CreateRotationY(XMConvertToRadians(220.0f))),
		Vector3::One * 15.0f);

	SpawnPickableItem(
		"GiantMiku",
		"../Assets/Models/Miku/", "Miku.obj",
		katamaryMat, coeff1,
		{ -150.0f, 0.0f, -200.0f },
		Quaternion::CreateFromRotationMatrix(
			Matrix::CreateRotationX(XMConvertToRadians(-90.0f))* Matrix::CreateRotationY(XMConvertToRadians(180.0f))),
		Vector3::One * 100.0f);


	SpawnPickableItem(
		"Floor",
		"../Assets/Models/", "cube.obj",
		whiteMat, coeff3,
		{ -2000.0f, -100.0f, -2000.0f },
		Quaternion::Identity,
		{ 100.0f, 1.0f, 100.0f },
		100.0f);


	this->orbitalCamController = new OrbitalCameraController(inputDevice, displayWin, 0.005f, 500.0f, 100.0f, katamary, 1000.0f,
		200.0f,  // minDistanceToBodySurface
		3000.0f // maxDistanceToBodySurface
	);

	cameraControllers.emplace_back(this->orbitalCamController);
	SetCameraController(1);

	Vector4 direction = { 1.0f, -0.5f, 1.0f, 0.0f };
	direction.Normalize();

	/*fountainPS = new ParticleSystemSorted(this, nullptr, Vector3::Up * 200.0f + Vector3::Backward * 200.0f + Vector3::Right * 500.0f, Quaternion::Identity, Vector3::One, camera);
	AddParticleSystem(fountainPS);*/
}

void KatamaryGame::SpawnPickableItem(
	const std::string& name,
	const std::string& modelDir, const std::string& modelName,
	Material* material, const PhongCoefficients& coeff,
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Quaternion& rotation,
	const DirectX::SimpleMath::Vector3& scale,
	float startScale)
{
	auto* object = new GameObject(name);
	auto* transform = object->GetTransform();
	transform->positionOffset = position;
	transform->rotation = rotation;
	transform->scale = scale;
	auto* model = object->AddComponent<Model<BoundingOrientedBox>>();
	model->Bind(modelDir, modelName, geometryPassShaderPath, depthShaderPath, startScale, material, coeff);
}

void KatamaryGame::Update()
{
	GameFramework::Update();

	if (fountainPS != nullptr)
	{
		for (int i = 0; i < 5; ++i)
		{
			ParticleSystemSorted::Particle p;
	
			Transform* transform = fountainPS->m_gameObject->GetTransform();
			Vector3 pos = transform->GetWorldMatrix().Translation();
			p.Position = { pos.x, pos.y, pos.z, 1.0f };

			Vector3 upVector = Vector3::Transform(Vector3::Up, transform->rotation);
			Vector3 rightVector = Vector3::Transform(Vector3::Right, transform->rotation);
			Vector3 forwardVector = Vector3::Transform(Vector3::Forward, transform->rotation);

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
