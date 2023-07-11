#include "d3dcompiler.h"
#include "WICTextureLoader.h"
#include "Material.h"
#include <iostream>
#include "DisplayWin.h"

#pragma comment(lib, "d3dcompiler.lib")


using namespace DirectX;

Material::Material(const LPCWSTR geometryPassShaderPath, const LPCWSTR shadowMappingShaderPath, Microsoft::WRL::ComPtr<ID3D11Device> device, DisplayWin* displayWin,
	const LPCWSTR diffuseTexturePath)
{
	// Create geometry pass resources shaders
	geometryPassVertexBC = nullptr;
	ID3DBlob* errorVertexCode = nullptr;

	HRESULT res = D3DCompileFromFile(geometryPassShaderPath,
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&geometryPassVertexBC,
		&errorVertexCode);

	CheckShaderCreationSuccess(res, errorVertexCode, geometryPassShaderPath);

	geometryPassPixelBC = nullptr;
	ID3DBlob* errorPixelCode = nullptr;

	res = D3DCompileFromFile(geometryPassShaderPath, nullptr /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &geometryPassPixelBC, &errorPixelCode);
	CheckShaderCreationSuccess(res, errorPixelCode, geometryPassShaderPath);

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

	res = D3DCompileFromFile(shadowMappingShaderPath,
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&shadowMappingVertexBC,
		&errorVertexCode);

	CheckShaderCreationSuccess(res, errorVertexCode, shadowMappingShaderPath);

	shadowMappingGeometryBC = nullptr;
	ID3DBlob* errorGeometryCode = nullptr;

	res = D3DCompileFromFile(shadowMappingShaderPath, nullptr /*macros*/, nullptr /*include*/, "GSMain", "gs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &shadowMappingGeometryBC, &errorGeometryCode);
	CheckShaderCreationSuccess(res, errorGeometryCode, shadowMappingShaderPath);

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

	CreateWICTextureFromFile(device.Get(), diffuseTexturePath, &texture, &textureView);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	device->CreateSamplerState(&samplerDesc, &pSampler);

	this->device = device;
	this->displayWin = displayWin;
}

Material::~Material()
{
}


void Material::CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName)
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
			MessageBox(displayWin->hWnd, shaderName, shaderName, MB_OK);
		}
	}
}