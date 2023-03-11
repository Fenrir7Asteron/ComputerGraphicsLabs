#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "GameFrameworkExports.h"

class DisplayWin;

class Material
{
public:
	GAMEFRAMEWORK_API Material();
	GAMEFRAMEWORK_API Material(const LPCWSTR vertexShaderPath, const LPCWSTR pixelShaderPath, Microsoft::WRL::ComPtr<ID3D11Device> device, DisplayWin* displayWin);
	GAMEFRAMEWORK_API virtual ~Material();

	ID3DBlob* vertexBC;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* pixelBC;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	DisplayWin* displayWin;

protected:
	GAMEFRAMEWORK_API void CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName);
};

