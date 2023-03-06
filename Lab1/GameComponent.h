#pragma once
#include <d3d11.h>

#include "GameFrameworkExports.h"
#include "SimpleMath.h"

class GameFramework;

class GameComponent
{
public:
	GAMEFRAMEWORK_API GameComponent(GameFramework* game,
		GameComponent* parent,
		DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero,
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::Identity,
		DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One);

	GAMEFRAMEWORK_API virtual void Update(float deltaTime) = 0;
	GAMEFRAMEWORK_API virtual void Draw() = 0;
	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta);
	GAMEFRAMEWORK_API virtual void Rotate(DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API virtual void RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle);
	GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix WorldTransformMatrix();
	GAMEFRAMEWORK_API virtual void UpdateWorldMatrix();
	GAMEFRAMEWORK_API virtual const DirectX::SimpleMath::Matrix& GetWorldMatrix();

	DirectX::SimpleMath::Vector3 positionOffset;
	DirectX::SimpleMath::Quaternion rotation;
	DirectX::SimpleMath::Vector3 scale;
	bool enabled;
	GameComponent* parent;

	

protected:
	GAMEFRAMEWORK_API void CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName);

	GameFramework* game_;
	DirectX::SimpleMath::Matrix objectToWorldMatrix_;
	bool worldMatrixIsDirty_;
};

