#include "Camera.h"
#include <d3d11.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera(float nearPlane, float farPlane, float fovY, int width, int height)
{
	this->nearZ = nearPlane;
	this->farZ = farPlane;
	this->fovY = fovY;
	this->width = width;
	this->height = height;
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix Camera::GetViewMatrix()
{
	Matrix T = Matrix::CreateTranslation(position);
	Matrix R = Matrix::CreateFromQuaternion(rotation);
	return (R * T).Invert();
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix Camera::GetProjectionMatrix()
{
	float aspect = width / height;
	return XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
}
