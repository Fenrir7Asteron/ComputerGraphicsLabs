#pragma once
#include "Component/GameComponent.h"
#include "Component/Render/RenderComponent.h"
#include <d3d11.h>
#include "PhongCoefficients.h"
#include "Model.h"
#include <vector>

class UnlitDiffuseMaterial;
class Vertex;
class PhysicalBoxComponent;
class PhysicalSphereComponent;
class PhysicalComponent;

class KatamaryBall : public GameComponent, public RenderComponent
{
public:
	KatamaryBall(GameObject* gameObject);
	void Bind(float radius, float moveSpeed, PhysicalSphereComponent* physicalComponent);

	// Inherited via GameComponent
	virtual void Update(float deltaTime) override;
	void Destroy() override;

	// Inherited via RenderComponent
	void DrawShadowMap() override;
	virtual void GeometryPass() override;

	void AttachObject(PhysicalComponent* other, float otherVolume);

	float GetOtherObjectVolume(PhysicalBoxComponent* other);
	float GetOtherObjectVolume(PhysicalSphereComponent* other);

	float Volume();
	void IncreaseSize(float sizeDelta);

	float radius;
	float targetVolume;
	float moveSpeed;
	PhysicalSphereComponent* m_physicalComponent;
};

