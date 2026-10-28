#include "PhysicalSphereComponent.h"
#include "Component/GameComponent.h"
#include "Component/GameObject.h"

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API void PhysicalSphereComponent::Bind(PhysicalLayer physicalLayer, float radius)
{
	this->physicalLayer;
}

GAMEFRAMEWORK_API void PhysicalSphereComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	m_gameObject->GetTransform()->Move(positionDelta);
	boundingSphere.Center = boundingSphere.Center + positionDelta;
}
