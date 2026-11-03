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

	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta);
	GAMEFRAMEWORK_API virtual void Rotate(DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API virtual void RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API virtual void ApplyTransform(DirectX::SimpleMath::Matrix m);
	GAMEFRAMEWORK_API virtual void SetParent(Transform* newParent);
	GAMEFRAMEWORK_API virtual void UpdateWorldMatrix();
	GAMEFRAMEWORK_API virtual const DirectX::SimpleMath::Matrix& GetWorldMatrix();

	GAMEFRAMEWORK_API int SubscribeToTransformChange(TransformChanged callback);
	GAMEFRAMEWORK_API void UnsubscribeFromTransformChange(int subscriberId);

	DirectX::SimpleMath::Vector3 positionOffset;
	DirectX::SimpleMath::Quaternion rotation;
	DirectX::SimpleMath::Vector3 scale;
	Transform* parent;
	std::vector<Transform*> children;

protected:
	DirectX::SimpleMath::Matrix m_objectToWorldMatrix;

private:
	GAMEFRAMEWORK_API void OnTransformChanged();

	std::map<int, TransformChanged> m_onTransformChangedCallbacks;
	int m_subscribersCount = 0;
};
