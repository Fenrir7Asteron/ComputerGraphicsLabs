#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"

GAMEFRAMEWORK_API struct LightConstantData
{
	DirectX::SimpleMath::Vector4 cameraPos;
	DirectX::SimpleMath::Vector4 lightParam1;
	DirectX::SimpleMath::Vector4 lightColor;
	DirectX::SimpleMath::Vector4 DSAIntensity;
};