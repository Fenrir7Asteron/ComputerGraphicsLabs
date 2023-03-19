#pragma once
#include "GameComponent.h"
#include "DirectXCollision.h"
#include "PhysicalLayer.h"

class GameFramework;

class PhysicalSphereComponent : public GameComponent
{
public:
	GAMEFRAMEWORK_API PhysicalSphereComponent(GameFramework* game,
		GameComponent* parent,
		PhysicalLayer physicalLayer = PhysicalLayer::Default,
		float radius = 1.0f,
		DirectX::SimpleMath::Matrix transform = DirectX::SimpleMath::Matrix::Identity,
		Material* material = nullptr);

	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta) override;

	DirectX::BoundingSphere boundingSphere;
	PhysicalLayer physicalLayer = PhysicalLayer::Default;
};