#pragma once
#include "GameComponent.h"
#include "DirectXCollision.h"
#include "PhysicalLayer.h"

class GameFramework;



class PhysicalBoxComponent : public GameComponent
{
public:
	GAMEFRAMEWORK_API PhysicalBoxComponent(GameFramework* game);
	GAMEFRAMEWORK_API PhysicalBoxComponent(GameFramework* game, PhysicalLayer physicalLayer);
	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta) override;

	DirectX::BoundingBox boundingBox;
	PhysicalLayer physicalLayer = PhysicalLayer::Default;
};