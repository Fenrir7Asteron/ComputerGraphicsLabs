#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3d11.h>
#include "GameComponent.h"
#include <vector>


class CelestialBody : public GameComponent
{
public:
	GAMEFRAMEWORK_API CelestialBody(GameFramework* game, float radius, int verticesNPerAxis, float distanceFromSun, DirectX::SimpleMath::Vector3 startDistanceAxis,
		float rotationPeriodInDays, float revolutionPeriodInDays,
		CelestialBody* revolutionOrigin,
		DirectX::SimpleMath::Vector3 rotationAxis,
		DirectX::SimpleMath::Vector3 revolutionAxis,
		DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero,
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::Identity,
		DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;

	ID3DBlob* vertexBC;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* pixelBC;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	std::vector<DirectX::XMFLOAT4> points;
	int pointsLen;
	std::vector<int> indices;
	int indicesLen;
	ID3D11RasterizerState* rastState;

	float distanceFromSun;
	float rotationSpeed;
	float revolutionSpeed;
	float radius;
	CelestialBody* revolutionOrigin;
	DirectX::SimpleMath::Vector3 rotationAxis;
	DirectX::SimpleMath::Vector3 revolutionAxis;
};

