#pragma once
#include "GameFramework.h"

class UnlitDiffuseMaterial;

class StaticBox : public GameComponent
{
public:
	GAMEFRAMEWORK_API StaticBox(GameFramework* game,
		DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero,
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::Identity,
		DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One,
		Material* material = nullptr);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;

	std::vector<DirectX::SimpleMath::Vector4> points;
	int pointsLen;
	std::vector<int> indices;
	int indicesLen;

	UnlitDiffuseMaterial* unlitDiffuseMaterial;
};