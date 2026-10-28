#pragma once
#include "GameFrameworkExports.h"
#include "Utils/SimpleMath.h"

class Camera
{
public:
	GAMEFRAMEWORK_API Camera(float nearPlane, float farPlane, float fovY, int width, int height);
	GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix GetViewMatrix();
	GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix GetProjectionMatrix();
	GAMEFRAMEWORK_API void SetOrthographic(bool isOthographic);

	float m_nearZ;
	float m_farZ;
	DirectX::SimpleMath::Vector3 m_position;
	DirectX::SimpleMath::Quaternion m_rotation;

private:
	float m_fovYRadians;
	int m_width;
	int m_height;
	bool m_isOrthographic;
};

