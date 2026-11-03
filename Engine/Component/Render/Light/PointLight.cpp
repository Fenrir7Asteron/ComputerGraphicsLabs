#include "PointLight.h"
#include "Model.h"

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API PointLight::PointLight(GameObject* gameObject) : GameComponent(gameObject)
{
}

GAMEFRAMEWORK_API void PointLight::Bind(
	Vector4 lightColor,
	float range, float diffuseIntensity, float specularIntensity,
	Model* lightMesh)
{
	this->lightColor = lightColor;
	this->range = range;
	this->diffuseIntensity = diffuseIntensity;
	this->specularIntensity = specularIntensity;
	this->lightMesh = lightMesh;
}

GAMEFRAMEWORK_API void PointLight::Update(float deltaTime)
{
}

GAMEFRAMEWORK_API void PointLight::Destroy()
{
}
