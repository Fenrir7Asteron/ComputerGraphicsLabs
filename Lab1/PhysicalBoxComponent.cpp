#include "PhysicalBoxComponent.h"
#include "GameComponent.h"

using namespace DirectX::SimpleMath;

PhysicalBoxComponent::PhysicalBoxComponent(GameFramework* game, GameComponent* parent, Vector3 position, Quaternion rotation, Vector3 scale, Material* material) : GameComponent(game, parent, position, rotation, scale, material) {}

PhysicalBoxComponent::PhysicalBoxComponent(GameFramework* game, GameComponent* parent, PhysicalLayer physicalLayer, Vector3 position, Quaternion rotation, Vector3 scale, Material* material) : GameComponent(game, parent, position, rotation, scale, material)
{
	this->physicalLayer = physicalLayer;
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	GameComponent::Move(positionDelta);
	boundingBox.Center = positionOffset;
}
