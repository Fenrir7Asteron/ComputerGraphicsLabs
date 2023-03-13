#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"

GAMEFRAMEWORK_API struct DirectionalLight
{
	DirectX::SimpleMath::Vector4 direction;
	DirectX::SimpleMath::Vector4 dirLightDiffuseIntensity;
	DirectX::SimpleMath::Vector4 dirLightSpecularIntensity;
	DirectX::SimpleMath::Vector4 dirLightAmbientIntensity;
};