#pragma once
#include "GameComponent.h"
#include "DirectXCollision.h"

class GameFramework;

class PhysicalBoxComponent : public GameComponent
{
public:
	GAMEFRAMEWORK_API PhysicalBoxComponent(GameFramework* game);
	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta) override;

	DirectX::BoundingBox boundingBox;
};