#include "CameraController.h"
#include <cmath>

using namespace DirectX::SimpleMath;

CameraController::CameraController()
{
}

GAMEFRAMEWORK_API void CameraController::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	if (camera == nullptr)
		return;

	camera->position += positionDelta;
}

GAMEFRAMEWORK_API void CameraController::Rotate(DirectX::SimpleMath::Vector3 axis, float angle)
{
	if (camera == nullptr)
		return;

	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * std::cos(axis.x);
	float qy = std::sin(angleHalf) * std::cos(axis.y);
	float qz = std::sin(angleHalf) * std::cos(axis.z);
	float qw = std::cos(angleHalf);
	Quaternion delta = { qx, qy, qz, qw };
	camera->rotation += delta;
}

GAMEFRAMEWORK_API void CameraController::SetCamera(Camera* camera)
{
	this->camera = camera;
}
