#pragma once
#include <d3d11.h>
#include "PhysicalSphereComponent.h"
#include "PhongCoefficients.h"
#include <vector>

class UnlitDiffuseMaterial;
class Vertex;

class KatamaryBall : public PhysicalSphereComponent
{
public:
	GAMEFRAMEWORK_API KatamaryBall(GameFramework* game, float radius, int verticesNPerAxis, float moveSpeed,
		const PhongCoefficients phongCoefficients,
		DirectX::SimpleMath::Matrix transform = DirectX::SimpleMath::Matrix::Identity,
		Material* material = nullptr,
		PhysicalLayer physicalLayer = PhysicalLayer::Default);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;
	void AttachObject(GameComponent* other);
	float GetOtherObjectVolume(GameComponent* other);
	float Volume();
	void IncreaseSize(float sizeDelta);

	std::vector<Vertex> points;
	std::vector<GameComponent*> attachedObjects;
	int verticesLen;
	std::vector<int> indices;
	int indicesLen;

	UnlitDiffuseMaterial* unlitDiffuseMaterial;
	PhongCoefficients phongCoefficients;

	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	ID3D11Buffer* constantMvpBuffer;
	ID3D11Buffer* constantPhongBuffer;

	float radius;
	float targetVolume;
	float moveSpeed;
};

