#define NOMINMAX
#define _USE_MATH_DEFINES
#include "KatamaryBall.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "Camera/Camera.h"
#include "Component/GameObject.h"
#include "Component/Physics/PhysicalBoxComponent.h"
#include "Component/Physics/PhysicalSphereComponent.h"
#include "Render/Debug/DebugRenderSysImpl.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <PhongConstantData.h>
#include <Vertex.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX;
using namespace DirectX::SimpleMath;

KatamaryBall::KatamaryBall(GameObject* gameObject) : GameComponent(gameObject)
{
}

void KatamaryBall::Bind(float radius, float moveSpeed, PhysicalSphereComponent* physicalComponent)
{
	this->radius = radius;
	this->m_physicalComponent = physicalComponent;
	this->moveSpeed = moveSpeed;
	this->targetVolume = Volume();
}

void KatamaryBall::Update(float deltaTime)
{
	/*float volume = Volume();
	if (volume + 0.001f < targetVolume)
	{
		float scaleDelta = std::min((float) std::pow((targetVolume - volume) / M_PI * (3.0f / 4.0f), 1.0f / 3.0f), deltaTime * 5.0f);
		IncreaseSize(scaleDelta);
	}*/

	Vector3 moveDir = Vector3::Zero;

	if (GameFramework::Instance->inputDevice->IsKeyDown(Keys::W))	moveDir += Vector3::Forward;
	if (GameFramework::Instance->inputDevice->IsKeyDown(Keys::A))	moveDir += Vector3::Left;
	if (GameFramework::Instance->inputDevice->IsKeyDown(Keys::S))	moveDir += Vector3::Backward;
	if (GameFramework::Instance->inputDevice->IsKeyDown(Keys::D))	moveDir += Vector3::Right;

	if (moveDir.LengthSquared() <= 0.001f)
		return;

	moveDir = Vector3::Transform(moveDir, GameFramework::Instance->camera->m_rotation);
	moveDir.y = 0.0f;

	moveDir.Normalize();

	Vector3 rotationAxis = moveDir.Cross(Vector3::Up);

	float moveDelta = moveSpeed * deltaTime;

	auto* transform = this->m_gameObject->GetTransform();
	transform->Move(moveDir * moveDelta);
	transform->RotateAxis(rotationAxis, -moveDelta / radius);

	GameComponent* other = GameFramework::Instance->Intersects(m_physicalComponent);
	if (other != nullptr)
	{
		PhysicalBoxComponent* boxModel = dynamic_cast<PhysicalBoxComponent*>(other);
		if (boxModel != nullptr)
		{
			float otherVolume = GetOtherObjectVolume(boxModel);
			if (targetVolume > otherVolume * 2.0f)
				AttachObject(boxModel, otherVolume);
		}
		else
		{
			PhysicalSphereComponent* sphereModel = dynamic_cast<PhysicalSphereComponent*>(other);
			if (sphereModel != nullptr)
			{
				float otherVolume = GetOtherObjectVolume(sphereModel);
				if (targetVolume > otherVolume * 2.0f)
					AttachObject(sphereModel, otherVolume);
			}
		}
	}
}

void KatamaryBall::AttachObject(PhysicalComponent* other, float otherVolume)
{
	other->m_gameObject->GetTransform()->SetParent(this->m_gameObject->GetTransform());
	other->m_drawBoundary = false;
	other->enabled = false;

	targetVolume += otherVolume;
}

float KatamaryBall::GetOtherObjectVolume(PhysicalBoxComponent* other)
{
	Vector3 extents = other->m_boundingShape.Extents;
	return extents.x * extents.y * extents.z * 8.0f;
}

float KatamaryBall::GetOtherObjectVolume(PhysicalSphereComponent* other)
{
	return (4.0f / 3.0f)* M_PI* std::pow(other->m_boundingShape.Radius, 3);
}

float KatamaryBall::Volume()
{
	return (4.0f / 3.0f) * M_PI * std::pow(this->radius, 3);
}

void KatamaryBall::IncreaseSize(float sizeDelta)
{
	this->radius += sizeDelta;

	auto* transform = m_gameObject->GetTransform();
	transform->SetScale(Vector3::One * sizeDelta);
	transform->Move(Vector3::Up * sizeDelta);
	m_physicalComponent->m_boundingShape.Center = m_physicalComponent->m_boundingShape.Center + Vector3::Up * sizeDelta;
	m_physicalComponent->m_boundingShape.Radius = this->radius;
}

void KatamaryBall::Destroy()
{
}

void KatamaryBall::DrawShadowMap()
{
}

void KatamaryBall::GeometryPass()
{
	GameFramework::Instance->debugRender->DrawSphere(m_physicalComponent->m_boundingShape.Radius, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(m_physicalComponent->m_boundingShape.Center), 16);
}
