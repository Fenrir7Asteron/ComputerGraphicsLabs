#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"

GAMEFRAMEWORK_API struct DirectionalLight
{
	DirectX::SimpleMath::Vector4 direction;
	DirectX::SimpleMath::Vector4 lightColor;
	float diffuseIntensity;
	float specularIntensity;
	float ambientIntensity;
};