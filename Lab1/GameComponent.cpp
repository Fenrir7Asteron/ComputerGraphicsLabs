#include <d3d11.h>
#include <iostream>

#include "GameComponent.h"
#include "GameFramework.h"

GameComponent::GameComponent(GameFramework* game, Vector3 position, Quaternion rotation, Vector3 scale)
{
	game_ = game;
	enabled = true;

	this->positionOffset = position;

	rotation.Normalize();
	this->rotation = rotation;

	this->scale = scale;

	Matrix rotationTransformMatrix =
	{
		1.0f - 2.0f * (rotation.y * rotation.y + rotation.z * rotation.z), 2.0f * (rotation.x * rotation.y + rotation.w * rotation.z)       , 2.0f * (rotation.x * rotation.z - rotation.w * rotation.y)       , 0.0f,
		2.0f * (rotation.x * rotation.y - rotation.w * rotation.z)       , 1.0f - 2.0f * (rotation.x * rotation.x + rotation.z * rotation.z), 2.0f * (rotation.y * rotation.z + rotation.w * rotation.x)       , 0.0f,
		2.0f * (rotation.x * rotation.z + rotation.w * rotation.y)       , 2.0f * (rotation.y * rotation.z - rotation.w * rotation.x)       , 1.0f - 2.0f * (rotation.x * rotation.x + rotation.y * rotation.y), 0.0f,
		0.0f                                                             , 0.0f                                                             , 0.0f                                                             , 1.0f,
	};

	this->objectToWorldMatrix = Matrix::CreateScale(scale) * rotationTransformMatrix * Matrix::CreateTranslation(position);
}

GAMEFRAMEWORK_API void GameComponent::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
	positionOffset += positionDelta;
	objectToWorldMatrix += Matrix::CreateTranslation(positionDelta);
}

GAMEFRAMEWORK_API DirectX::SimpleMath::Matrix GameComponent::WorldTransformMatrix()
{
	
	
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
