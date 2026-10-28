#include "Transform.h"

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API Transform::Transform(GameObject* gameObject) 	: GameComponent(gameObject)
{
	this->positionOffset = Vector3::One;
	this->rotation = Quaternion::Identity;
	this->scale = Vector3::One;
	UpdateWorldMatrix();
}

GAMEFRAMEWORK_API void Transform::Update(float deltaTime)
{
}

GAMEFRAMEWORK_API void Transform::Destroy()
{
	m_onTransformChangedCallbacks.clear();
	children.clear();
}

GAMEFRAMEWORK_API void Transform::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	positionOffset += positionDelta;
	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::Rotate(DirectX::SimpleMath::Vector3 axis, float angle)
{
	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion delta = { qx, qy, qz, qw };

	rotation *= delta;
	rotation.Normalize();

	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle)
{
	Vector3 newPosition = positionOffset - point;

	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion rot = { qx, qy, qz, qw };
	rot.Normalize();

	rotation = rotation * rot;

	newPosition = Vector3::Transform(newPosition, rot);
	positionOffset = newPosition + point;

	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::ApplyTransform(DirectX::SimpleMath::Matrix m)
{
	auto newTransform = GetWorldMatrix() * m;
	newTransform.Decompose(scale, rotation, positionOffset);

	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::SetParent(Transform* newParent)
{
	auto newParentM = newParent->GetWorldMatrix();
	auto newParentInverseM = newParentM.Invert();
	ApplyTransform(newParentInverseM);

	// Exclude this Transform from old parent's children list
	if (parent != nullptr)
		parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this));

	newParent->children.push_back(this);

	this->parent = newParent;
	
	
	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::UpdateWorldMatrix()
{
	this->m_objectToWorldMatrix = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(positionOffset);

	if (parent != nullptr)
		this->m_objectToWorldMatrix = this->m_objectToWorldMatrix * parent->GetWorldMatrix();
}

GAMEFRAMEWORK_API const Matrix& Transform::GetWorldMatrix()
{
	Transform* cur = this;
	while (cur != nullptr)
	{
		cur = cur->parent;
	}

	UpdateWorldMatrix();

	return m_objectToWorldMatrix;
}

GAMEFRAMEWORK_API int Transform::SubscribeToTransformChange(TransformChanged callback)
{
	m_onTransformChangedCallbacks[m_subscribersCount] = (std::move(callback));
	return m_subscribersCount++;
}

GAMEFRAMEWORK_API void Transform::UnsubscribeFromTransformChange(int subscriberId)
{
	if (subscriberId >= m_subscribersCount || subscriberId < 0)
		return;

	m_onTransformChangedCallbacks.erase(m_onTransformChangedCallbacks.find(m_subscribersCount - 1));
	m_subscribersCount--;
}

GAMEFRAMEWORK_API void Transform::OnTransformChanged()
{
	for (int i = 0; i < m_onTransformChangedCallbacks.size(); ++i)
	{
		m_onTransformChangedCallbacks[i]();
	}

	for (auto child : children)
	{
		child->OnTransformChanged();
	}
}
