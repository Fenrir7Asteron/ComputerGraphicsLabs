#include <d3d11.h>
#include <iostream>

#include "GameComponent.h"
#include "GameFramework.h"

GameComponent::GameComponent(GameFramework* game)
{
	game_ = game;
}

GAMEFRAMEWORK_API void GameComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	positionOffset += positionDelta;
}

void GameComponent::CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName)
{
	if (FAILED(res)) {
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode) {
			char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

			std::cout << compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(*(game_->displayWin->hWnd), shaderName, shaderName, MB_OK);
		}
	}
}
