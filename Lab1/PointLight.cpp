#include "PointLight.h"

using namespace DirectX::SimpleMath;

PointLight::PointLight()
{
}

PointLight::PointLight(
	Vector3 lightPos, Vector4 lightColor,
	float range, float diffuseIntensity, float specularIntensity,
	Model<BoundingSphere>* lightMesh
)
{
	this->lightPos = lightPos;
	this->lightColor = lightColor;
	this->range = range;
	this->diffuseIntensity = diffuseIntensity;
	this->specularIntensity = specularIntensity;
	this->lightMesh = lightMesh;
}
