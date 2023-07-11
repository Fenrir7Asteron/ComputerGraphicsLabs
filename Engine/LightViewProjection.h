#pragma once
#include "GlobalSettings.h"
#include "SimpleMath.h"

struct LightViewProjection
{
	DirectX::SimpleMath::Matrix viewProjection[GlobalSettings::CASCADES_COUNT];
	float distances[GlobalSettings::CASCADES_COUNT];
};