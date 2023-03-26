#pragma once
#include <d3d11.h>
#include "PhongCoefficients.h"
#include "Model.h"
#include <vector>

class UnlitDiffuseMaterial;
class Vertex;

template <class T>
class KatamaryBall : public Model<T>
{
public:
	KatamaryBall(GameFramework* game, float radius, float moveSpeed,
		const PhongCoefficients phongCoefficients,
		const std::string modelDir,
		const std::string modelName,
		const LPCWSTR shaderPath, const LPCWSTR depthShaderPath,
		float startScale,
		DirectX::SimpleMath::Matrix transform = DirectX::SimpleMath::Matrix::Identity,
		Material* material = nullptr,
		PhysicalLayer physicalLayer = PhysicalLayer::Default);

	// Inherited via GameComponent
	virtual void Update(float deltaTime) override;
	virtual void Draw() override;

	template <class U>
	void AttachObject(Model<U>* other);

	float GetOtherObjectVolume(Model<BoundingOrientedBox>* other);
	float GetOtherObjectVolume(Model<BoundingSphere>* other);

	float Volume();
	void IncreaseSize(float sizeDelta);

	std::vector<GameComponent*> attachedObjects;
	float radius;
	float targetVolume;
	float moveSpeed;
};

