#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"

GAMEFRAMEWORK_API struct PhongConstantData
{
	DirectX::SimpleMath::Vector4 cameraPosition;

	DirectX::SimpleMath::Vector4 direction;

	DirectX::SimpleMath::Vector4 dirLightDiffuseCoefficient;
	DirectX::SimpleMath::Vector4 dirLightDiffuseIntensity;

	DirectX::SimpleMath::Vector4 dirLightSpecularCoefficient_alpha; // {kD.x, kD.y, kD.z, a}
	DirectX::SimpleMath::Vector4 dirLightSpecularIntensity;

	DirectX::SimpleMath::Vector4 dirLightAmbientCoefficient;
	DirectX::SimpleMath::Vector4 dirLightAmbientIntensity;
};