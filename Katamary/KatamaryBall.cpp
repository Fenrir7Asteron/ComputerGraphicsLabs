#define NOMINMAX
#define _USE_MATH_DEFINES
#include "KatamaryBall.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "UnlitDiffuseMaterial.h"
#include "DebugRenderSysImpl.h"
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

KatamaryBall<BoundingSphere>::KatamaryBall(GameFramework* game, float radius, float moveSpeed,
	const PhongCoefficients phongCoefficients,
	const std::string modelDir,
	const std::string modelName,
	const LPCWSTR shaderPath, float startScale,
	DirectX::SimpleMath::Matrix transform, Material* material, PhysicalLayer physicalLayer)
	: Model<BoundingSphere>(game, nullptr, transform, modelDir, modelName, shaderPath, startScale, material, phongCoefficients, physicalLayer)
{
	this->radius = radius;
	this->scale = Vector3::One * radius;
	this->moveSpeed = moveSpeed;
	this->targetVolume = Volume();

	this->Move(Vector3::Up * radius);

	this->physicalComponent.boundingShape.Radius *= radius;
}

KatamaryBall<BoundingOrientedBox>::KatamaryBall(GameFramework* game, float radius, float moveSpeed,
	const PhongCoefficients phongCoefficients,
	const std::string modelDir,
	const std::string modelName,
	const LPCWSTR shaderPath, float startScale,
	DirectX::SimpleMath::Matrix transform, Material* material, PhysicalLayer physicalLayer)
	: Model<BoundingOrientedBox>(game, nullptr, transform, modelDir, modelName, shaderPath, startScale, material, phongCoefficients, physicalLayer)
{
	this->radius = radius;
	this->scale = Vector3::One * radius;
	this->moveSpeed = moveSpeed;
	this->targetVolume = Volume();

	this->Move(Vector3::Up * radius);

	this->physicalComponent.boundingShape.Extents = this->physicalComponent.boundingShape.Extents * (Vector3::One * radius);
}

template <class T>
void KatamaryBall<T>::Update(float deltaTime)
{
	float volume = Volume();
	if (volume + 0.001f < targetVolume)
	{
		float scaleDelta = std::min((float) std::pow((targetVolume - volume) / M_PI * (3.0f / 4.0f), 1.0f / 3.0f), deltaTime * 5.0f);
		IncreaseSize(scaleDelta);
	}

	Vector3 moveDir = Vector3::Zero;

	if (this->game_->inputDevice->IsKeyDown(Keys::W))	moveDir += Vector3::Forward;
	if (this->game_->inputDevice->IsKeyDown(Keys::A))	moveDir += Vector3::Left;
	if (this->game_->inputDevice->IsKeyDown(Keys::S))	moveDir += Vector3::Backward;
	if (this->game_->inputDevice->IsKeyDown(Keys::D))	moveDir += Vector3::Right;

	if (moveDir.LengthSquared() <= 0.001f)
		return;

	moveDir = Vector3::Transform(moveDir, this->game_->camera->rotation);
	moveDir.y = 0.0f;

	moveDir.Normalize();

	Vector3 rotationAxis = moveDir.Cross(Vector3::Up);

	float moveDelta = moveSpeed * deltaTime;

	this->Move(moveDir * moveDelta);
	this->Rotate(rotationAxis, -moveDelta / radius);

	for (auto attached : attachedObjects)
	{
		attached->Move(moveDir * moveDelta);
		attached->RotateAroundPoint(this->GetWorldMatrix().Translation(), rotationAxis, -moveDelta / radius);
	}

	GameComponent* other = this->game_->Intersects(&(this->physicalComponent));
	if (other != nullptr)
	{
		Model<BoundingOrientedBox>* boxModel = dynamic_cast<Model<BoundingOrientedBox>*>(other);
		if (boxModel != nullptr)
		{
			float otherVolume = GetOtherObjectVolume(boxModel);
			if (volume > otherVolume * 2.0f)
				AttachObject(boxModel);
		}
		else
		{
			Model<BoundingSphere>* sphereModel = dynamic_cast<Model<BoundingSphere>*>(other);
			if (sphereModel != nullptr)
			{
				float otherVolume = GetOtherObjectVolume(sphereModel);
				if (volume > otherVolume * 2.0f)
					AttachObject(sphereModel);
			}
		}
	}
}

template <class T>
template <class U>
void KatamaryBall<T>::AttachObject(Model<U>* other)
{
	attachedObjects.push_back(other);
	other->enabled = false;

	targetVolume += GetOtherObjectVolume(other);
}

template <class T>
float KatamaryBall<T>::GetOtherObjectVolume(Model<BoundingOrientedBox>* other)
{
	Vector3 extents = other->physicalComponent.boundingShape.Extents;
	return extents.x * extents.y * extents.z * 8.0f;
}

template <class T>
float KatamaryBall<T>::GetOtherObjectVolume(Model<BoundingSphere>* other)
{
	return (4.0f / 3.0f)* M_PI* std::pow(other->physicalComponent.boundingShape.Radius, 3);
}

template <class T>
float KatamaryBall<T>::Volume()
{
	return (4.0f / 3.0f) * M_PI * std::pow(this->radius, 3);
}

void KatamaryBall<BoundingSphere>::IncreaseSize(float sizeDelta)
{
	this->radius += sizeDelta;
	this->scale += Vector3::One * sizeDelta;
	this->positionOffset += Vector3::Up * sizeDelta;
	this->physicalComponent.boundingShape.Center = this->physicalComponent.boundingShape.Center + Vector3::Up * sizeDelta;
	this->physicalComponent.boundingShape.Radius = this->radius;

	for (auto attached : attachedObjects)
	{
		Vector3 moveDir = attached->GetWorldMatrix().Translation() - this->GetWorldMatrix().Translation();
		moveDir.Normalize();

		attached->Move(moveDir * sizeDelta);
	}
}

void KatamaryBall<BoundingOrientedBox>::IncreaseSize(float sizeDelta)
{
	this->radius += sizeDelta;
	this->scale += Vector3::One * sizeDelta;
	this->positionOffset += Vector3::Up * sizeDelta;
	this->physicalComponent.boundingShape.Center = this->physicalComponent.boundingShape.Center + Vector3::Up * sizeDelta;
	this->physicalComponent.boundingShape.Extents = Vector3::One * this->radius;

	for (auto attached : attachedObjects)
	{
		Vector3 moveDir = attached->GetWorldMatrix().Translation() - this->GetWorldMatrix().Translation();
		moveDir.Normalize();

		attached->Move(moveDir * sizeDelta);
	}
}

void KatamaryBall<BoundingSphere>::Draw()
{
	Model<BoundingSphere>::Draw();
	this->game_->debugRender->DrawSphere(this->physicalComponent.boundingShape.Radius, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(this->physicalComponent.boundingShape.Center), 16);
}

void KatamaryBall<BoundingOrientedBox>::Draw()
{
	Model<BoundingOrientedBox>::Draw();
	this->game_->debugRender->DrawSphere(this->physicalComponent.boundingShape.Extents.x, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(this->physicalComponent.boundingShape.Center), 16);
}

