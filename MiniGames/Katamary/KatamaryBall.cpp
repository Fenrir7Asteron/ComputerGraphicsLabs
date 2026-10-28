#define NOMINMAX
#define _USE_MATH_DEFINES
#include "KatamaryBall.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "Camera/Camera.h"
#include "Component/GameObject.h"
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

KatamaryBall<BoundingSphere>::KatamaryBall(GameObject* gameObject) : Model<BoundingSphere>(gameObject)
{
}

void KatamaryBall<BoundingSphere>::Bind(float radius, float moveSpeed,
	const PhongCoefficients phongCoefficients,
	const std::string modelDir,
	const std::string modelName,
	const LPCWSTR geometryPassShaderPath, const LPCWSTR depthShaderPath,
	float startScale,
	Material* material, PhysicalLayer physicalLayer)
{
	Model<BoundingSphere>::Bind(modelDir, modelName, geometryPassShaderPath, depthShaderPath, startScale, material, phongCoefficients, physicalLayer);

	this->radius = radius;

	auto* transform = m_gameObject->GetTransform();
	transform->SetScale(Vector3::One * radius);
	this->moveSpeed = moveSpeed;
	this->targetVolume = Volume();

	transform->Move(Vector3::Up * radius);

	this->m_physicalComponent.boundingShape.Radius *= radius;
}

void KatamaryBall<BoundingOrientedBox>::Bind(float radius, float moveSpeed,
	const PhongCoefficients phongCoefficients,
	const std::string modelDir,
	const std::string modelName,
	const LPCWSTR geometryPassShaderPath, const LPCWSTR depthShaderPath,
	float startScale,
	Material* material, PhysicalLayer physicalLayer)
{
	Model<BoundingOrientedBox>::Bind(modelDir, modelName, geometryPassShaderPath, depthShaderPath, startScale, material, phongCoefficients, physicalLayer);

	this->radius = radius;
	this->moveSpeed = moveSpeed;
	this->targetVolume = Volume();

	this->m_gameObject->GetTransform()->Move(Vector3::Up * radius);

	this->m_physicalComponent.boundingShape.Extents = this->m_physicalComponent.boundingShape.Extents * (Vector3::One * radius);
}

template <class T>
void KatamaryBall<T>::Update(float deltaTime)
{
	Model<T>::Update(deltaTime);

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

	GameComponent* other = GameFramework::Instance->Intersects(&(this->m_physicalComponent));
	if (other != nullptr)
	{
		Model<BoundingOrientedBox>* boxModel = dynamic_cast<Model<BoundingOrientedBox>*>(other);
		if (boxModel != nullptr)
		{
			float otherVolume = GetOtherObjectVolume(boxModel);
			if (targetVolume > otherVolume * 2.0f)
				AttachObject(boxModel);
		}
		else
		{
			Model<BoundingSphere>* sphereModel = dynamic_cast<Model<BoundingSphere>*>(other);
			if (sphereModel != nullptr)
			{
				float otherVolume = GetOtherObjectVolume(sphereModel);
				if (targetVolume > otherVolume * 2.0f)
					AttachObject(sphereModel);
			}
		}
	}
}

template <class T>
template <class U>
void KatamaryBall<T>::AttachObject(Model<U>* other)
{
	other->m_gameObject->GetTransform()->SetParent(this->m_gameObject->GetTransform());
	other->m_drawBoundary = false;
	other->m_physicalComponent.enabled = false;

	targetVolume += GetOtherObjectVolume(other);
}

template <class T>
float KatamaryBall<T>::GetOtherObjectVolume(Model<BoundingOrientedBox>* other)
{
	Vector3 extents = other->m_physicalComponent.boundingShape.Extents;
	return extents.x * extents.y * extents.z * 8.0f;
}

template <class T>
float KatamaryBall<T>::GetOtherObjectVolume(Model<BoundingSphere>* other)
{
	return (4.0f / 3.0f)* M_PI* std::pow(other->m_physicalComponent.boundingShape.Radius, 3);
}

template <class T>
float KatamaryBall<T>::Volume()
{
	return (4.0f / 3.0f) * M_PI * std::pow(this->radius, 3);
}

void KatamaryBall<BoundingSphere>::IncreaseSize(float sizeDelta)
{
	this->radius += sizeDelta;

	auto* transform = m_gameObject->GetTransform();
	transform->SetScale(Vector3::One * sizeDelta);
	transform->Move(Vector3::Up * sizeDelta);
	this->m_physicalComponent.boundingShape.Center = this->m_physicalComponent.boundingShape.Center + Vector3::Up * sizeDelta;
	this->m_physicalComponent.boundingShape.Radius = this->radius;
}

void KatamaryBall<BoundingOrientedBox>::IncreaseSize(float sizeDelta)
{
	this->radius += sizeDelta;

	auto* transform = m_gameObject->GetTransform();
	transform->SetScale(Vector3::One * sizeDelta);
	transform->Move(Vector3::Up * sizeDelta);
	this->m_physicalComponent.boundingShape.Center = this->m_physicalComponent.boundingShape.Center + Vector3::Up * sizeDelta;
	this->m_physicalComponent.boundingShape.Extents = Vector3::One * this->radius;
}

void KatamaryBall<BoundingSphere>::GeometryPass()
{
	Model<BoundingSphere>::GeometryPass();
	GameFramework::Instance->debugRender->DrawSphere(this->m_physicalComponent.boundingShape.Radius, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(this->m_physicalComponent.boundingShape.Center), 16);
}

void KatamaryBall<BoundingOrientedBox>::GeometryPass()
{
	Model<BoundingOrientedBox>::GeometryPass();
	GameFramework::Instance->debugRender->DrawSphere(this->m_physicalComponent.boundingShape.Extents.x, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(this->m_physicalComponent.boundingShape.Center), 16);
}

