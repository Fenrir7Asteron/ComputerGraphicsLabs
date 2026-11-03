#pragma once
#include "Utils/SimpleMath.h"
#include "GameFrameworkExports.h"

class PhongCoefficients
{
public:
	DirectX::SimpleMath::Vector4 dirLightDiffuseCoefficient;
	DirectX::SimpleMath::Vector4 dirLightSpecularCoefficient_alpha; // {kD.x, kD.y, kD.z, a}
	DirectX::SimpleMath::Vector4 dirLightAmbientCoefficient;
};