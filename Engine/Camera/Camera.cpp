#include "Camera.h"
#include <d3d11.h>
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera(float nearPlane, float farPlane, float fovYDegrees, int width, int height)
{
	this->m_nearZ = nearPlane;
	this->m_farZ = farPlane;
	this->m_fovYRadians = XMConvertToRadians(fovYDegrees);
	this->m_width = width;
	this->m_height = height;

	m_rotation = Quaternion::LookRotation(Vector3::Forward, Vector3::Up);
	SetOrthographic(false);
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix Camera::GetViewMatrix()
{
	Vector3 forward = Vector3::Transform(Vector3::Forward, m_rotation);
	Vector3 up = Vector3::Transform(Vector3::Up, m_rotation);

	return XMMatrixLookAtLH(m_position, m_position + forward, up);
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix Camera::GetProjectionMatrix()
{
	if (this->m_isOrthographic)
	{
		// return orthographic projection matrix
		return XMMatrixOrthographicLH(m_width, m_height, m_nearZ, m_farZ);	
	}
	else 
	{
		// return perspective projection matrix
		float aspect = m_width / m_height;
		return XMMatrixPerspectiveFovLH(m_fovYRadians, aspect, m_nearZ, m_farZ);
	}
}

GAMEFRAMEWORK_API void Camera::SetOrthographic(bool isOrthographic)
{
	if (this->m_isOrthographic == isOrthographic)
		return;

	this->m_isOrthographic = isOrthographic;
}
