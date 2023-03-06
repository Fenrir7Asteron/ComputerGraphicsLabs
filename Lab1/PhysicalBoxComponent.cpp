#include "PhysicalBoxComponent.h"
#include "GameComponent.h"

using namespace DirectX::SimpleMath;

PhysicalBoxComponent::PhysicalBoxComponent(GameFramework* game, Vector3 position, Quaternion rotation, Vector3 scale) : GameComponent(game, position, rotation, scale) {}

PhysicalBoxComponent::PhysicalBoxComponent(GameFramework* game, PhysicalLayer physicalLayer, Vector3 position, Quaternion rotation, Vector3 scale) : GameComponent(game, position, rotation, scale)
{
	this->physicalLayer = physicalLayer;
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	GameComponent::Move(positionDelta);
	boundingBox.Center = positionOffset;
}
