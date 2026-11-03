#pragma once
#include "GameFrameworkExports.h"

class RenderComponent
{
public:
	GAMEFRAMEWORK_API virtual void DrawShadowMap() = 0;
	GAMEFRAMEWORK_API virtual void GeometryPass() = 0;

	bool castShadows = true;
};

