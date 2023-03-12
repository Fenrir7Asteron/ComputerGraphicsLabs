#include <d3d11.h>
#include <iostream>

#include "GameComponent.h"
#include "GameFramework.h"

using namespace DirectX::SimpleMath;

GameComponent::GameComponent(GameFramework* game, GameComponent* parent, Vector3 position, Quaternion rotation, Vector3 scale, Material* material)
{
	game_ = game;
	enabled = true;

	this->positionOffset = position;

	rotation.Normalize();
	this->rotation = rotation;

	this->scale = scale;
	this->parent = parent;

	UpdateWorldMatrix();

	this->material = material;
}

GameComponent::GameComponent(GameFramework* game, GameComponent* parent,
	Matrix transform, Material* material)
{
	game_ = game;
	enabled = true;

	transform.Decompose(this->scale, this->rotation, this->positionOffset);
	this->parent = parent;

	UpdateWorldMatrix();

	this->material = material;
}

GAMEFRAMEWORK_API void GameComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	positionOffset += positionDelta;
	worldMatrixIsDirty_ = true;
}

GAMEFRAMEWORK_API void GameComponent::Rotate(DirectX::SimpleMath::Vector3 axis, float angle)
{
	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion delta = { qx, qy, qz, qw };

	rotation *= delta;
	rotation.Normalize();

	worldMatrixIsDirty_ = true;
}

GAMEFRAMEWORK_API void GameComponent::RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle)
{
	Vector3 newPosition = positionOffset - point;

	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion rot = { qx, qy, qz, qw };
	rot.Normalize();

	newPosition = Vector3::Transform(newPosition, rot);
	positionOffset = newPosition + point;
	worldMatrixIsDirty_ = true;
}

GAMEFRAMEWORK_API void GameComponent::UpdateWorldMatrix()
{
	this->objectToWorldMatrix_ = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(positionOffset);

	worldMatrixIsDirty_ = false;

	if (parent != nullptr)
		this->objectToWorldMatrix_ = this->objectToWorldMatrix_ * Matrix::CreateTranslation(parent->GetWorldMatrix().Translation());
}

GAMEFRAMEWORK_API const Matrix& GameComponent::GetWorldMatrix()
{
	bool isDirty = false;
	GameComponent* cur = this;
	while (cur != nullptr)
	{
		if (cur->worldMatrixIsDirty_)
			isDirty = true;

		cur = cur->parent;
	}

	if (isDirty)
	{
		UpdateWorldMatrix();
	}

	return objectToWorldMatrix_;
}
