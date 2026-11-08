#pragma once
#include "GameFrameworkExports.h"
#include <d3d11.h>
#include <string>

namespace RenderAPI::DirectX11
{
	class ShaderAPIUtils
	{
	public:
		GAMEFRAMEWORK_API void static CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const std::wstring& shaderName);
	};
}

