#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "GameFrameworkExports.h"
#include <string>

class DisplayWin;

class Material
{
public:
	GAMEFRAMEWORK_API Material(const std::wstring& geometryPassShaderPath, const std::wstring& shadowMappingShaderPath, DisplayWin* displayWin,
		const std::wstring& diffuseTexturePath);

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
};

