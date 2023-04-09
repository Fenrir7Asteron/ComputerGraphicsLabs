#pragma once
#include "GameFrameworkExports.h"
#include "SimpleMath.h"
#include "Model.h"

class GameFramework;

class PointLight
{
public:
	GAMEFRAMEWORK_API PointLight();

	GAMEFRAMEWORK_API PointLight(
		DirectX::SimpleMath::Vector3 lightPos,
		DirectX::SimpleMath::Vector4 lightColor,
		float range,
		float diffuseIntensity, float specularIntensity,
		Model<BoundingSphere>* lightMesh
	);

	DirectX::SimpleMath::Vector3 lightPos;
	DirectX::SimpleMath::Vector4 lightColor;
	float range;
	float diffuseIntensity;
	float specularIntensity;
	float ambientIntensity;
	Model<BoundingSphere>* lightMesh;
};

