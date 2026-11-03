#include "PhysicalBoxComponent.h"
#include "PhysicalSphereComponent.h"
#include "Component/GameComponent.h"
#include "Component/GameObject.h"
#include "Render/Debug/DebugRenderSysImpl.h"
#include "Render/RenderAPI/DirectX11/DirectX11RenderAPI.h"
#include "GameFramework.h"

PhysicalBoxComponent::PhysicalBoxComponent(GameObject* gameObject) : PhysicalComponent(gameObject)
{
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::Bind(Model* model, PhysicalLayer physicalLayer)
{
    PhysicalComponent::Bind(model, physicalLayer);

    DirectX::BoundingOrientedBox::CreateFromBoundingBox(m_boundingShapeNoTransform, aabbBoundingBox);
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::UpdateCollider()
{
    m_boundingShape = m_boundingShapeNoTransform;

    auto* transform = m_gameObject->GetTransform();
    m_boundingShape.Transform(m_boundingShape, transform->GetWorldMatrix());
}

GAMEFRAMEWORK_API void PhysicalBoxComponent::GeometryPass()
{
    if (!m_drawBoundary)
        return;
      
    RenderAPI::DirectX11::DirectX11RenderAPI::Instance->debugRender->DrawOrientedBoundingBox(m_boundingShape, Matrix::Identity);
}

GAMEFRAMEWORK_API bool PhysicalBoxComponent::Intersects(PhysicalBoxComponent* other)
{
    if (other->enabled == false || other == this)
        return false;

    return m_boundingShape.Intersects(other->m_boundingShape);
}

GAMEFRAMEWORK_API bool PhysicalBoxComponent::Intersects(PhysicalSphereComponent* other)
{
    if (other->enabled == false)
        return false;

    return m_boundingShape.Intersects(other->m_boundingShape);
}

