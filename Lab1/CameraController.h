#pragma once
#include "Camera.h"


class CameraController
{
public:
	GAMEFRAMEWORK_API CameraController();
	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta);
	GAMEFRAMEWORK_API virtual void Rotate(DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API virtual void SetCamera(Camera* camera);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) = 0;

	Camera* camera;
};
