#include "DirectionalLight.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API DirectionalLight::DirectionalLight()
{
}

GAMEFRAMEWORK_API DirectionalLight::DirectionalLight(
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Vector4 direction,
	Vector4 lightColor,
	float diffuseIntensity, float specularIntensity, float ambientIntensity, 
	float nearPlane, float farPlane,
	int shadowMapWidth,	int shadowMapHeight)
{
	this->direction = direction;
	this->lightColor = lightColor;
	this->diffuseIntensity = diffuseIntensity;
	this->specularIntensity = specularIntensity;
	this->ambientIntensity = ambientIntensity;


	// Create shadow map resources
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	shadowMapDesc.Height = static_cast<UINT>(512);
	shadowMapDesc.Width = static_cast<UINT>(512);

	HRESULT res = device->CreateTexture2D(
		&shadowMapDesc,
		nullptr,
		&shadowMap
	);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	res = device->CreateDepthStencilView(
		shadowMap,
		&depthStencilViewDesc,
		&shadowDepthView
	);

	res = device->CreateShaderResourceView(
		shadowMap,
		&shaderResourceViewDesc,
		&shadowResourceView
	);


	D3D11_SAMPLER_DESC comparisonSamplerDesc;
	ZeroMemory(&comparisonSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

	// Point filtered shadows can be faster, and may be a good choice when
	// rendering on hardware with lower feature levels. This sample has a
	// UI option to enable/disable filtering so you can see the difference
	// in quality and speed.

	device->CreateSamplerState(
		&comparisonSamplerDesc,
		&comparisonSampler
	);

	D3D11_RASTERIZER_DESC shadowRenderStateDesc;
	ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
	shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
	shadowRenderStateDesc.DepthClipEnable = true;

	res = device->CreateRasterizerState(&shadowRenderStateDesc, &shadowRastState);


	D3D11_BUFFER_DESC viewProjectionConstantBufferDesc = {};
	viewProjectionConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewProjectionConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewProjectionConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewProjectionConstantBufferDesc.MiscFlags = 0;
	viewProjectionConstantBufferDesc.StructureByteStride = 0;
	viewProjectionConstantBufferDesc.ByteWidth = sizeof(Matrix);

	viewProjection = XMMatrixOrthographicLH(shadowMapWidth, shadowMapHeight, nearPlane, farPlane);

	D3D11_SUBRESOURCE_DATA viewProjectionConstantBufferData = {};
	viewProjectionConstantBufferData.pSysMem = &viewProjection;
	viewProjectionConstantBufferData.SysMemPitch = 0;
	viewProjectionConstantBufferData.SysMemSlicePitch = 0;

	device->CreateBuffer(
		&viewProjectionConstantBufferDesc,
		&viewProjectionConstantBufferData,
		&constantLightViewProjectionBuffer
	);
}