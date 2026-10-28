#pragma once
#include "Component/GameComponent.h"
#include "DirectXCollision.h"
#include "Physics/PhysicalLayer.h"

class GameFramework;

class PhysicalBoxComponent : public GameComponent
{
public:
	GAMEFRAMEWORK_API void Bind(PhysicalLayer physicalLayer = PhysicalLayer::Default);

	GAMEFRAMEWORK_API void Move(DirectX::SimpleMath::Vector3 positionDelta);

	DirectX::BoundingOrientedBox boundingBox;
	PhysicalLayer physicalLayer = PhysicalLayer::Default;
};