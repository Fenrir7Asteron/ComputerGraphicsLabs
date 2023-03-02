#pragma once
#include "GameFrameworkExports.h"

class Camera
{
public:
	float near_;
	float far_;
	float bottom_;
	float top_;
	int width_;
	int height_;

	GAMEFRAMEWORK_API Camera(float near, float far, float bottom, float top, int width, int height);
};

