#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"

GAMEFRAMEWORK_API struct PhongCoefficients
{
	DirectX::SimpleMath::Vector4 dirLightDiffuseCoefficient;
	DirectX::SimpleMath::Vector4 dirLightSpecularCoefficient_alpha; // {kD.x, kD.y, kD.z, a}
	DirectX::SimpleMath::Vector4 dirLightAmbientCoefficient;
};