#pragma once
#include <d3d11.h>
#include "GameComponent.h"
#include <vector>

class UnlitDiffuseMaterial;

class KatamaryBall : public GameComponent
{
public:
	GAMEFRAMEWORK_API KatamaryBall(GameFramework* game, float radius, int verticesNPerAxis,
		DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero,
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::Identity,
		DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One,
		Material* material = nullptr);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;

	std::vector<DirectX::XMFLOAT4> points;
	int verticesLen;
	std::vector<int> indices;
	int indicesLen;

	UnlitDiffuseMaterial* unlitDiffuseMaterial;

	float radius;
};

