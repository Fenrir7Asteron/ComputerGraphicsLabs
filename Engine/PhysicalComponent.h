#pragma once
#include "GameComponent.h"
#include "DirectXCollision.h"
#include "PhysicalLayer.h"

template <class T>
class PhysicalComponent
{
public:
	GAMEFRAMEWORK_API PhysicalComponent();

	GAMEFRAMEWORK_API PhysicalComponent(GameComponent* parent,
		PhysicalLayer physicalLayer = PhysicalLayer::Default,
		DirectX::SimpleMath::Matrix transform = DirectX::SimpleMath::Matrix::Identity);

	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta);
	GAMEFRAMEWORK_API virtual void Rotate(DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API virtual void RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle);

	T boundingShape;
	PhysicalLayer physicalLayer = PhysicalLayer::Default;
	GameComponent* parent;
	DirectX::SimpleMath::Matrix transform;
};
