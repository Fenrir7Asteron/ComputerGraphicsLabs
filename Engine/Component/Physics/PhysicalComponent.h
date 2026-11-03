#pragma once
#include "Component/GameComponent.h"
#include "Component/Render/RenderComponent.h"
#include "DirectXCollision.h"
#include "Physics/PhysicalLayer.h"

class Node;
class Model;
class PhysicalBoxComponent;
class PhysicalSphereComponent;

class PhysicalComponent : public GameComponent, public RenderComponent
{
public:
	GAMEFRAMEWORK_API PhysicalComponent(GameObject* gameObject);

	GAMEFRAMEWORK_API virtual void Bind(Model* model, PhysicalLayer physicalLayer = PhysicalLayer::Default);

	GAMEFRAMEWORK_API virtual bool Intersects(PhysicalBoxComponent* other) = 0;
	GAMEFRAMEWORK_API virtual bool Intersects(PhysicalSphereComponent* other) = 0;

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Destroy() override;

	// Inherited via RenderComponent
	GAMEFRAMEWORK_API void DrawShadowMap() override;
	GAMEFRAMEWORK_API virtual void GeometryPass() = 0;

	virtual void UpdateCollider() = 0;
	GAMEFRAMEWORK_API void UpdateAABBBoxBorders(const Node* pNode, DirectX::SimpleMath::Matrix accumulatedTransform);

	PhysicalLayer physicalLayer = PhysicalLayer::Default;
	bool m_drawBoundary = true;

protected:
	DirectX::BoundingBox aabbBoundingBox;

private:
	GAMEFRAMEWORK_API void InvalidateCollider();

	bool m_invalidateAABB = false;
	bool m_invalidateCollider = false;
	int m_subscriptionId = -1;

	DirectX::SimpleMath::Vector4 m_aabbMinPos;
	DirectX::SimpleMath::Vector4 m_aabbMaxPos;
};
