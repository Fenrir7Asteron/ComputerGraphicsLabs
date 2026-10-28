#pragma once
#include "Component/GameComponent.h"
#include<map>

using TransformChanged = std::function<void()>;

class Transform : public GameComponent
{
public:
	GAMEFRAMEWORK_API Transform(GameObject* gameObject);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Destroy() override;

	GAMEFRAMEWORK_API DirectX::SimpleMath::Vector3 GetPosition();
	GAMEFRAMEWORK_API DirectX::SimpleMath::Quaternion GetRotation();
	GAMEFRAMEWORK_API DirectX::SimpleMath::Vector3 GetScale();
	GAMEFRAMEWORK_API virtual const DirectX::SimpleMath::Matrix& GetWorldMatrix();
	GAMEFRAMEWORK_API virtual const Transform* GetParent();
	GAMEFRAMEWORK_API virtual const std::vector<Transform*>& GetChildren();

	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta);
	GAMEFRAMEWORK_API virtual void RotateAxis(DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API virtual void Rotate(DirectX::SimpleMath::Quaternion quaternion);
	GAMEFRAMEWORK_API virtual void RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API virtual void SetScale(DirectX::SimpleMath::Vector3 scale);
	GAMEFRAMEWORK_API virtual void ApplyTransform(DirectX::SimpleMath::Matrix m);
	GAMEFRAMEWORK_API virtual void SetParent(Transform* newParent);

	GAMEFRAMEWORK_API int SubscribeToTransformChange(TransformChanged callback);
	GAMEFRAMEWORK_API void UnsubscribeFromTransformChange(int subscriberId);

private:
	GAMEFRAMEWORK_API virtual void UpdateWorldMatrix();
	GAMEFRAMEWORK_API void OnTransformChanged();

	DirectX::SimpleMath::Vector3 m_positionOffset;
	DirectX::SimpleMath::Quaternion m_rotation;
	DirectX::SimpleMath::Vector3 m_scale;
	DirectX::SimpleMath::Matrix m_objectToWorldMatrix;
	Transform* m_parent;
	std::vector<Transform*> m_children;

	std::map<int, TransformChanged> m_onTransformChangedCallbacks;
	int m_subscribersCount = 0;
	bool m_isDirty;
};
