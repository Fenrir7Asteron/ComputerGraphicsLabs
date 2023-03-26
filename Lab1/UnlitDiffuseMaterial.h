#pragma once
#include "Material.h"
class UnlitDiffuseMaterial : public Material
{
public:
	GAMEFRAMEWORK_API UnlitDiffuseMaterial(const LPCWSTR vertexShaderPath, const LPCWSTR pixelShaderPath, const LPCWSTR depthShaderPath,
		Microsoft::WRL::ComPtr<ID3D11Device> device, DisplayWin* displayWin, const LPCWSTR diffuseTexturePath);

	ID3D11Resource* texture;
	ID3D11ShaderResourceView* textureView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;

	ID3DBlob* geometryDepthBC;
	ID3D11GeometryShader* geometryDepthShader;
	ID3D11InputLayout* shadowLayout;
};

