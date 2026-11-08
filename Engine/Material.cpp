#include "d3dcompiler.h"
#include "Render/Texture/Loader/WICTextureLoader.h"
#include "Render/RenderAPI/DirectX11/DirectX11RenderAPI.h"
#include "Render/RenderAPI/DirectX11/ShaderAPIUtils.h"
#include "Material.h"
#include "GameFramework.h"
#include <iostream>
#include "DisplayWin.h"

#pragma comment(lib, "d3dcompiler.lib")


using namespace DirectX;

Material::Material(const std::wstring& geometryPassShaderPath, const std::wstring& shadowMappingShaderPath, DisplayWin* displayWin,
	const std::wstring& diffuseTexturePath)
{
	auto& device = RenderAPI::DirectX11::DirectX11RenderAPI::Instance->device;

	// Create geometry pass resources shaders
	geometryPassVertexBC = nullptr;
	ID3DBlob* errorVertexCode = nullptr;

	HRESULT res = D3DCompileFromFile(geometryPassShaderPath.c_str(),
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&geometryPassVertexBC,
		&errorVertexCode);

	RenderAPI::DirectX11::ShaderAPIUtils::CheckShaderCreationSuccess(res, errorVertexCode, geometryPassShaderPath);

	geometryPassPixelBC = nullptr;
	ID3DBlob* errorPixelCode = nullptr;

	res = D3DCompileFromFile(geometryPassShaderPath.c_str(), nullptr /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &geometryPassPixelBC, &errorPixelCode);
	RenderAPI::DirectX11::ShaderAPIUtils::CheckShaderCreationSuccess(res, errorPixelCode, geometryPassShaderPath);

	res = device->CreateVertexShader(
		geometryPassVertexBC->GetBufferPointer(),
		geometryPassVertexBC->GetBufferSize(),
		nullptr, &geometryPassVertexShader);

	if (FAILED(res))
	{
		return;
	}

	res = device->CreatePixelShader(
		geometryPassPixelBC->GetBufferPointer(),
		geometryPassPixelBC->GetBufferSize(),
		nullptr, &geometryPassPixelShader);

	if (FAILED(res))
	{
		return;
	}


	// Create shadow mapping pass resources shaders
	shadowMappingVertexBC = nullptr;
	errorVertexCode = nullptr;

	res = D3DCompileFromFile(shadowMappingShaderPath.c_str(),
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shadowMappingVertexBC,
		&errorVertexCode);

	RenderAPI::DirectX11::ShaderAPIUtils::CheckShaderCreationSuccess(res, errorVertexCode, shadowMappingShaderPath);

	shadowMappingGeometryBC = nullptr;
	ID3DBlob* errorGeometryCode = nullptr;

	res = D3DCompileFromFile(shadowMappingShaderPath.c_str(), nullptr /*macros*/, nullptr /*include*/, "GSMain", "gs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shadowMappingGeometryBC, &errorGeometryCode);
	RenderAPI::DirectX11::ShaderAPIUtils::CheckShaderCreationSuccess(res, errorGeometryCode, shadowMappingShaderPath);

	res = device->CreateVertexShader(
		shadowMappingVertexBC->GetBufferPointer(),
		shadowMappingVertexBC->GetBufferSize(),
		nullptr, &shadowMappingVertexShader);

	if (FAILED(res))
	{
		return;
	}

	res = device->CreateGeometryShader(
		shadowMappingGeometryBC->GetBufferPointer(),
		shadowMappingGeometryBC->GetBufferSize(),
		nullptr, &shadowMappingGeometryShader);

	if (FAILED(res))
	{
		return;
	}


	D3D11_INPUT_ELEMENT_DESC inputElements1[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	D3D11_INPUT_ELEMENT_DESC inputElements2[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};


	device->CreateInputLayout(
		inputElements1,
		4,
		geometryPassVertexBC->GetBufferPointer(),
		geometryPassVertexBC->GetBufferSize(),
		&geometryLayout);

	device->CreateInputLayout(
		inputElements2,
		1,
		shadowMappingVertexBC->GetBufferPointer(),
		shadowMappingVertexBC->GetBufferSize(),
		&shadowLayout);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = device->CreateRasterizerState(&rastDesc, &rastState);

	CreateWICTextureFromFile(device.Get(), diffuseTexturePath.c_str(), &texture, &textureView);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	device->CreateSamplerState(&samplerDesc, &pSampler);
}

Material::~Material()
{
}
