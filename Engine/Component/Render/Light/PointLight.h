#pragma once
#include "Component/GameComponent.h"
#include "PointLightConstantData.h"

class GameFramework;
class Model;

class PointLight : public GameComponent
{
public:
	GAMEFRAMEWORK_API PointLight(GameObject* gameObject);

	GAMEFRAMEWORK_API void Bind(
		DirectX::SimpleMath::Vector4 lightColor,
		float range,
		float diffuseIntensity, float specularIntensity,
		Model* lightMesh
	);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Destroy() override;

	DirectX::SimpleMath::Vector4 lightColor;
	float range;
	float diffuseIntensity;
	float specularIntensity;
	float ambientIntensity;
	Model* lightMesh;

	PointLightConstantData pointLightMvp;
};

