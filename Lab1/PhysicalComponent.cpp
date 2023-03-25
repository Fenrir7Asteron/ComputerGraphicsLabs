#include "PhysicalComponent.h"
#include "GameComponent.h"
#include <cmath>

using namespace DirectX;
using namespace DirectX::SimpleMath;

template<class T>
GAMEFRAMEWORK_API PhysicalComponent<T>::PhysicalComponent()
{
}

template GAMEFRAMEWORK_API PhysicalComponent<BoundingOrientedBox>::PhysicalComponent();
template GAMEFRAMEWORK_API PhysicalComponent<BoundingSphere>::PhysicalComponent();

GAMEFRAMEWORK_API PhysicalComponent<BoundingOrientedBox>::PhysicalComponent(GameComponent* parent, PhysicalLayer physicalLayer, DirectX::SimpleMath::Matrix transform)
{
	this->physicalLayer = physicalLayer;
	this->parent = parent;

	Vector3 center;
	Vector3 scale;
	Quaternion rotation;
	transform.Decompose(scale, rotation, center);
	this->boundingShape = BoundingOrientedBox(center, scale, rotation);
}

GAMEFRAMEWORK_API PhysicalComponent<BoundingSphere>::PhysicalComponent(GameComponent* parent, PhysicalLayer physicalLayer, DirectX::SimpleMath::Matrix transform)
{
	this->physicalLayer = physicalLayer;
	this->parent = parent;

	Vector3 center;
	Vector3 scale;
	Quaternion rotation;
	transform.Decompose(scale, rotation, center);
	
	float radius = scale.x;
	if (scale.y > radius)
		radius = scale.y;
	if (scale.z > radius)
		radius = scale.z;

	this->boundingShape = BoundingSphere(center, radius);
}

template<class T>
GAMEFRAMEWORK_API void PhysicalComponent<T>::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	boundingShape.Center = boundingShape.Center + positionDelta;
}

template<class T>
GAMEFRAMEWORK_API void PhysicalComponent<T>::Rotate(DirectX::SimpleMath::Vector3 axis, float angle)
{
	return;
}

GAMEFRAMEWORK_API void PhysicalComponent<BoundingOrientedBox>::Rotate(DirectX::SimpleMath::Vector3 axis, float angle)
{
	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion delta = { qx, qy, qz, qw };

	Quaternion newOrientation = boundingShape.Orientation;
	newOrientation *= delta;
	newOrientation.Normalize();

	boundingShape.Orientation = newOrientation;
}

template<class T>
GAMEFRAMEWORK_API void PhysicalComponent<T>::RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle)
{
	return;
}

GAMEFRAMEWORK_API void PhysicalComponent<BoundingOrientedBox>::RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle)
{
	Vector3 newPosition = boundingShape.Center - point;

	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion rot = { qx, qy, qz, qw };
	rot.Normalize();

	Quaternion newOrientation = boundingShape.Orientation;
	boundingShape.Orientation = newOrientation * rot;

	newPosition = Vector3::Transform(newPosition, rot);
	boundingShape.Center = newPosition + point;
}
