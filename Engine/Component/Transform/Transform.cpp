#include "Transform.h"

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API Transform::Transform(GameObject* gameObject) 	: GameComponent(gameObject)
{
	this->m_positionOffset = Vector3::Zero;
	this->m_rotation = Quaternion::Identity;
	this->m_scale = Vector3::One;

	OnTransformChanged();

	UpdateWorldMatrix();
}

GAMEFRAMEWORK_API void Transform::Update(float deltaTime)
{
}

GAMEFRAMEWORK_API void Transform::Destroy()
{
	m_onTransformChangedCallbacks.clear();
	m_children.clear();
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Vector3 Transform::GetPosition()
{
	return m_positionOffset;
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Quaternion Transform::GetRotation()
{
	return m_rotation;
}

DirectX::SimpleMath::Vector3 Transform::GetScale()
{
	return m_scale;
}

GAMEFRAMEWORK_API void Transform::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	m_positionOffset += positionDelta;
	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::RotateAxis(DirectX::SimpleMath::Vector3 axis, float angle)
{
	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion delta = { qx, qy, qz, qw };

	m_rotation *= delta;
	m_rotation.Normalize();

	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::Rotate(DirectX::SimpleMath::Quaternion quaternion)
{
	m_rotation *= quaternion;
	m_rotation.Normalize();
	
	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle)
{
	Vector3 newPosition = m_positionOffset - point;

	float angleHalf = angle / 2;
	float qx = std::sin(angleHalf) * axis.x;
	float qy = std::sin(angleHalf) * axis.y;
	float qz = std::sin(angleHalf) * axis.z;
	float qw = std::cos(angleHalf);
	Quaternion rot = { qx, qy, qz, qw };
	rot.Normalize();

	m_rotation = m_rotation * rot;

	newPosition = Vector3::Transform(newPosition, rot);
	m_positionOffset = newPosition + point;

	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::SetScale(DirectX::SimpleMath::Vector3 scale)
{
	m_scale = scale;

	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::ApplyTransform(DirectX::SimpleMath::Matrix m)
{
	auto newTransform = GetWorldMatrix() * m;
	newTransform.Decompose(m_scale, m_rotation, m_positionOffset);

	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::SetParent(Transform* newParent)
{
	auto newParentM = newParent->GetWorldMatrix();
	auto newParentInverseM = newParentM.Invert();
	ApplyTransform(newParentInverseM);

	// Exclude this Transform from old parent's children list
	if (m_parent != nullptr)
		m_parent->m_children.erase(std::remove(m_parent->m_children.begin(), m_parent->m_children.end(), this));

	newParent->m_children.push_back(this);

	this->m_parent = newParent;
	
	OnTransformChanged();
}

GAMEFRAMEWORK_API void Transform::UpdateWorldMatrix()
{
	if (!m_isDirty)
		return;

	m_objectToWorldMatrix = Matrix::CreateScale(m_scale) * Matrix::CreateFromQuaternion(m_rotation) * Matrix::CreateTranslation(m_positionOffset);

	if (m_parent != nullptr)
		m_objectToWorldMatrix = m_objectToWorldMatrix * m_parent->GetWorldMatrix();

	m_isDirty = false;
	
}

GAMEFRAMEWORK_API const Matrix& Transform::GetWorldMatrix()
{
	Transform* cur = this;
	while (cur != nullptr)
	{
		cur = cur->m_parent;
	}

	UpdateWorldMatrix();

	return m_objectToWorldMatrix;
}

GAMEFRAMEWORK_API const Transform* Transform::GetParent()
{
	return m_parent;
}

GAMEFRAMEWORK_API const std::vector<Transform*>& Transform::GetChildren()
{
	return m_children;
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
	m_isDirty = true;

	for (int i = 0; i < m_onTransformChangedCallbacks.size(); ++i)
	{
		m_onTransformChangedCallbacks[i]();
	}

	for (auto child : m_children)
	{
		child->OnTransformChanged();
	}
}
