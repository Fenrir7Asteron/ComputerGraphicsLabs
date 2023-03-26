#pragma once
#include "Material.h"
class UnlitDiffuseMaterial : public Material
{
public:
	GAMEFRAMEWORK_API UnlitDiffuseMaterial(const LPCWSTR vertexShaderPath, const LPCWSTR pixelShaderPath, const LPCWSTR vertexDepthShaderPath,
		Microsoft::WRL::ComPtr<ID3D11Device> device, DisplayWin* displayWin, const LPCWSTR diffuseTexturePath);

	ID3D11Resource* texture;
	ID3D11ShaderResourceView* textureView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;

	ID3DBlob* vertexDepthBC;
	ID3D11VertexShader* vertexDepthShader;
	ID3D11InputLayout* shadowLayout;
};

