#pragma once
#include "GameFrameworkExports.h"
#include "SimpleMath.h"

class Camera
{
public:
	float nearZ;
	float farZ;
	float fovY;
	int width;
	int height;
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Quaternion rotation;

	GAMEFRAMEWORK_API Camera(float nearPlane, float farPlane, float fovY, int width, int height);
	GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix GetViewMatrix();
	GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix GetProjectionMatrix();
};

