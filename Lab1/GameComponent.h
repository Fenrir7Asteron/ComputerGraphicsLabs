#pragma once
#include <d3d11.h>

#include "GameFrameworkExports.h"
#include "SimpleMath.h"

class GameFramework;
using namespace DirectX::SimpleMath;

class GameComponent
{
public:
	GAMEFRAMEWORK_API GameComponent(GameFramework* game, Vector3 position, Quaternion rotation, Vector3 scale);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) = 0;
	GAMEFRAMEWORK_API virtual void Draw() = 0;
	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta);
	GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix WorldTransformMatrix();

	Vector3 positionOffset;
	Quaternion rotation;
	Vector3 scale;
	Matrix objectToWorldMatrix;
	bool enabled;

protected:
	GAMEFRAMEWORK_API void CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName);

	GameFramework* game_;
	
};

