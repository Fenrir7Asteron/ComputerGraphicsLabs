#pragma once
#include "Component/GameComponent.h"
#include "DirectXCollision.h"
#include "Physics/PhysicalLayer.h"

class GameFramework;

class PhysicalSphereComponent : public GameComponent
{
public:
	GAMEFRAMEWORK_API void Bind(
		PhysicalLayer physicalLayer = PhysicalLayer::Default,
		float radius = 1.0f);

	GAMEFRAMEWORK_API void Move(DirectX::SimpleMath::Vector3 positionDelta);

	DirectX::BoundingSphere boundingSphere;
	PhysicalLayer physicalLayer = PhysicalLayer::Default;
};