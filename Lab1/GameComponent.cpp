#include <d3d11.h>
#include <iostream>

#include "GameComponent.h"
#include "Game.h"

GameComponent::GameComponent(Game* game)
{
	game_ = game;
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
			MessageBox(game_->hWnd, shaderName, shaderName, MB_OK);
		}
	}
}
