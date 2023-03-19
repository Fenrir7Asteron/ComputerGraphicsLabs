#include "PhysicalSphereComponent.h"
#include "GameComponent.h"

using namespace DirectX::SimpleMath;

PhysicalSphereComponent::PhysicalSphereComponent(GameFramework* game, GameComponent* parent, PhysicalLayer physicalLayer, float radius, DirectX::SimpleMath::Matrix transform, Material* material) : GameComponent(game, parent, transform, material)
{
	this->physicalLayer;
}

GAMEFRAMEWORK_API void PhysicalSphereComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	GameComponent::Move(positionDelta);
	boundingSphere.Center = positionOffset;
}
