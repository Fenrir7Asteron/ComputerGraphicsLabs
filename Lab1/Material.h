#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "GameFrameworkExports.h"

class DisplayWin;

class Material
{
public:
	GAMEFRAMEWORK_API Material(const LPCWSTR geometryPassShaderPath, const LPCWSTR shadowMappingShaderPath, Microsoft::WRL::ComPtr<ID3D11Device> device, DisplayWin* displayWin,
		const LPCWSTR diffuseTexturePath);

	GAMEFRAMEWORK_API virtual ~Material();

	ID3DBlob* geometryPassVertexBC;
	ID3D11VertexShader* geometryPassVertexShader;
	ID3DBlob* geometryPassPixelBC;
	ID3D11PixelShader* geometryPassPixelShader;

	ID3DBlob* shadowMappingVertexBC;
	ID3DBlob* shadowMappingGeometryBC;
	ID3D11VertexShader* shadowMappingVertexShader;
	ID3D11GeometryShader* shadowMappingGeometryShader;

	ID3D11Resource* texture;
	ID3D11ShaderResourceView* textureView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;


	ID3D11InputLayout* geometryLayout;
	ID3D11InputLayout* shadowLayout;
	ID3D11RasterizerState* rastState;

	Microsoft::WRL::ComPtr<ID3D11Device> device;

	DisplayWin* displayWin;

protected:
	GAMEFRAMEWORK_API void CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName);
};

