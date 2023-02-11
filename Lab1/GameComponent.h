#pragma once
#include "GameFrameworkExports.h"

class GameFramework;

class GameComponent
{
public:
	GAMEFRAMEWORK_API GameComponent(GameFramework* game);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) = 0;
	GAMEFRAMEWORK_API virtual void Draw() = 0;

protected:
	GAMEFRAMEWORK_API void CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName);

	GameFramework* game_;
};

