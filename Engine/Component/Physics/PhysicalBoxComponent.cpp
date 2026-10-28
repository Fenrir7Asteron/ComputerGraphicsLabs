#include "PhysicalBoxComponent.h"
#include "Component/GameComponent.h"
#include "Component/GameObject.h"

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API void PhysicalBoxComponent::Bind(PhysicalLayer physicalLayer)
{
	this->physicalLayer = physicalLayer;
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	m_gameObject->GetTransform()->Move(positionDelta);
	boundingBox.Center = boundingBox.Center + positionDelta;
}
