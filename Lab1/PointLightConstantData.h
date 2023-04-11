#pragma once
#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct PointLightConstantData
{
	Matrix worldMatrix;
	Matrix viewMatrix;
	Matrix projectionMatrix;
};