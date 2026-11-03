#pragma once
#include "Utils/SimpleMath.h"
#include "GameFrameworkExports.h"

struct LightConstantData
{
	DirectX::SimpleMath::Vector4 cameraPos;
	DirectX::SimpleMath::Vector4 lightParam1;
	DirectX::SimpleMath::Vector4 lightColor;
	DirectX::SimpleMath::Vector4 DSAIntensity;
	DirectX::SimpleMath::Matrix CameraView;
};