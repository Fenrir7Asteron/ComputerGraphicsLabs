#include "PhysicalBoxComponent.h"
#include "PhysicalSphereComponent.h"
#include "Component/GameComponent.h"
#include "Component/GameObject.h"
#include "Render/Debug/DebugRenderSysImpl.h"
#include "Render/RenderAPI/DirectX11/DirectX11RenderAPI.h"
#include "GameFramework.h"

PhysicalSphereComponent::PhysicalSphereComponent(GameObject* gameObject) : PhysicalComponent(gameObject)
{
}

GAMEFRAMEWORK_API void PhysicalSphereComponent::Bind(Model* model, PhysicalLayer physicalLayer)
{
    PhysicalComponent::Bind(model, physicalLayer);

    DirectX::BoundingSphere::CreateFromBoundingBox(m_boundingShapeNoTransform, aabbBoundingBox);
    m_boundingShapeNoTransform.Radius = aabbBoundingBox.Extents.x;
}

GAMEFRAMEWORK_API void PhysicalSphereComponent::UpdateCollider()
{
    m_boundingShape = m_boundingShapeNoTransform;

    auto* transform = m_gameObject->GetTransform();
    m_boundingShape.Transform(m_boundingShape, transform->GetWorldMatrix());
}

GAMEFRAMEWORK_API void PhysicalSphereComponent::GeometryPass()
{
    if (!m_drawBoundary)
        return;

    RenderAPI::DirectX11::DirectX11RenderAPI::Instance->debugRender->DrawSphere(m_boundingShape.Radius, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(m_boundingShape.Center), 16);
}

GAMEFRAMEWORK_API bool PhysicalSphereComponent::Intersects(PhysicalBoxComponent* other)
{
    if (other->enabled == false)
        return false;

    return m_boundingShape.Intersects(other->m_boundingShape);
}

GAMEFRAMEWORK_API bool PhysicalSphereComponent::Intersects(PhysicalSphereComponent* other)
{
    if (other->enabled == false || other == this)
        return false;

    return m_boundingShape.Intersects(other->m_boundingShape);
}
