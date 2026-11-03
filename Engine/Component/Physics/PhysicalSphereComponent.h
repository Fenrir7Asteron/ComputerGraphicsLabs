#pragma once
#include "PhysicalComponent.h"

class GameFramework;

class PhysicalSphereComponent : public PhysicalComponent
{
public:
	GAMEFRAMEWORK_API PhysicalSphereComponent(GameObject* gameObject);

	// Inherited via PhysicalComponent
	GAMEFRAMEWORK_API void Bind(Model* model, PhysicalLayer physicalLayer = PhysicalLayer::Default) override;
	GAMEFRAMEWORK_API void UpdateCollider() override;
	GAMEFRAMEWORK_API void GeometryPass() override;
	GAMEFRAMEWORK_API bool Intersects(PhysicalBoxComponent* other) override;
	GAMEFRAMEWORK_API bool Intersects(PhysicalSphereComponent* other) override;

	DirectX::BoundingSphere m_boundingShape;

private:
	DirectX::BoundingSphere m_boundingShapeNoTransform;
};