#include "PhysicalBoxComponent.h"
#include "GameComponent.h"

PhysicalBoxComponent::PhysicalBoxComponent(GameFramework* game) : GameComponent(game)
{
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	GameComponent::Move(positionDelta);
	boundingBox.Center = boundingBox.Center + positionDelta;
}
