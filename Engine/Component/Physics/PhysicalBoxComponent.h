#pragma once
#include "PhysicalComponent.h"

class GameFramework;

class PhysicalBoxComponent : public PhysicalComponent
{
public:
	GAMEFRAMEWORK_API PhysicalBoxComponent(GameObject* gameObject);

	// Inherited via PhysicalComponent
	GAMEFRAMEWORK_API void Bind(Model* model, PhysicalLayer physicalLayer = PhysicalLayer::Default) override;
	GAMEFRAMEWORK_API void UpdateCollider() override;
	GAMEFRAMEWORK_API void GeometryPass() override;
	GAMEFRAMEWORK_API bool Intersects(PhysicalBoxComponent* other) override;
	GAMEFRAMEWORK_API bool Intersects(PhysicalSphereComponent* other) override;

	DirectX::BoundingOrientedBox m_boundingShape;

private:
	DirectX::BoundingOrientedBox m_boundingShapeNoTransform;
};
