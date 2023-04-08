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
		DirectX::SimpleMath::Vector4 lightPos,
		DirectX::SimpleMath::Vector4 lightColor,
		float range, float attenuation,
		float diffuseIntensity, float specularIntensity, float ambientIntensity,
		Model<BoundingSphere>* lightMesh
	);

	DirectX::SimpleMath::Vector4 lightPos;
	DirectX::SimpleMath::Vector4 lightColor;
	float range;
	float attenuation;
	float diffuseIntensity;
	float specularIntensity;
	float ambientIntensity;
	Model<BoundingSphere>* lightMesh;
};

