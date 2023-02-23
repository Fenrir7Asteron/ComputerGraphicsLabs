#include "PhysicalBoxComponent.h"
#include "GameComponent.h"

PhysicalBoxComponent::PhysicalBoxComponent(GameFramework* game) : GameComponent(game) {}

PhysicalBoxComponent::PhysicalBoxComponent(GameFramework* game, PhysicalLayer physicalLayer = PhysicalLayer::Default) : GameComponent(game)
{
	this->physicalLayer = physicalLayer;
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	GameComponent::Move(positionDelta);
	boundingBox.Center = positionOffset;
}
