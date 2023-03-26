#pragma once
#include "GlobalSettings.h"
#include "SimpleMath.h"

struct LightViewProjection
{
	DirectX::SimpleMath::Matrix view[GlobalSettings::CASCADES_COUNT];
	DirectX::SimpleMath::Matrix projection[GlobalSettings::CASCADES_COUNT];
	float distances[GlobalSettings::CASCADES_COUNT];
};