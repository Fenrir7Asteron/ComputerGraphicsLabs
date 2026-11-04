#include "ShaderAPIUtils.h"
#include "GameFramework.h"
#include "DisplayWin.h"
#include <iostream>

namespace RenderAPI::DirectX11
{
	GAMEFRAMEWORK_API void ShaderAPIUtils::CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName)
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
				MessageBox(GameFramework::Instance->displayWin->hWnd, shaderName, shaderName, MB_OK);
			}
		}
	}
}