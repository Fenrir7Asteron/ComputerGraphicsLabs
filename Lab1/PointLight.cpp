#include "PointLight.h"

using namespace DirectX::SimpleMath;

PointLight::PointLight()
{
}

PointLight::PointLight(
	Vector4 lightPos, Vector4 lightColor,
	float range, float attenuation, float diffuseIntensity, float specularIntensity, float ambientIntensity,
	Model<BoundingSphere>* lightMesh
)
{
	this->lightPos = lightPos;
	this->lightColor = lightColor;
	this->range = range;
	this->attenuation = attenuation;
	this->diffuseIntensity = diffuseIntensity;
	this->specularIntensity = specularIntensity;
	this->ambientIntensity = ambientIntensity;
	this->lightMesh = lightMesh;
}
