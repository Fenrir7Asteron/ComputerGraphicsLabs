#include "Camera.h"
#include <d3d11.h>
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera(float nearPlane, float farPlane, float fovYDegrees, int width, int height)
{
	this->nearZ = nearPlane;
	this->farZ = farPlane;
	this->fovYRadians = XMConvertToRadians(fovYDegrees);
	this->width = width;
	this->height = height;

	rotation = Quaternion::LookRotation(Vector3::Forward, Vector3::Up);
	SetOrthographic(false);
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix Camera::GetViewMatrix()
{
	Vector3 forward = Vector3::Transform(Vector3::Forward, rotation);
	Vector3 up = Vector3::Transform(Vector3::Up, rotation);

	return XMMatrixLookAtLH(position, position + forward, up);
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix Camera::GetProjectionMatrix()
{
	if (this->isOrthographic)
	{
		// return orthographic projection matrix
		return XMMatrixOrthographicLH(width, height, nearZ, farZ);	
	}
	else 
	{
		// return perspective projection matrix
		float aspect = width / height;
		return XMMatrixPerspectiveFovLH(fovYRadians, aspect, nearZ, farZ);
	}
}

GAMEFRAMEWORK_API void Camera::SetOrthographic(bool isOrthographic)
{
	if (this->isOrthographic == isOrthographic)
		return;

	this->isOrthographic = isOrthographic;
}
