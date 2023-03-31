#define NOMINMAX
#define _USE_MATH_DEFINES

#include "Camera.h"
#include "CameraController.h"
#include "GameFramework.h"
#include "DirectionalLight.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API DirectionalLight::DirectionalLight()
{
}

GAMEFRAMEWORK_API DirectionalLight::DirectionalLight(
	GameFramework* game,
	Vector4 lightDir,
	Vector4 lightColor,
	float diffuseIntensity, float specularIntensity, float ambientIntensity,
	Camera* renderCamera,
	float shadowDistance,
	int shadowMapWidth,	int shadowMapHeight)
{
	this->game = game;
	
	this->direction = lightDir;
	this->direction.Normalize();

	this->lightColor = lightColor;
	this->diffuseIntensity = diffuseIntensity;
	this->specularIntensity = specularIntensity;
	this->ambientIntensity = ambientIntensity;
	this->renderCamera = renderCamera;
	this->shadowDistance = shadowDistance;


	// Create shadow map resources
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = GlobalSettings::CASCADES_COUNT;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	shadowMapDesc.Height = static_cast<UINT>(shadowMapWidth);
	shadowMapDesc.Width = static_cast<UINT>(shadowMapHeight);

	HRESULT res = game->device->CreateTexture2D(
		&shadowMapDesc,
		nullptr,
		&shadowMap
	);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;
	depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthStencilViewDesc.Texture2DArray.ArraySize = GlobalSettings::CASCADES_COUNT;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.ArraySize = GlobalSettings::CASCADES_COUNT;

	res = game->device->CreateDepthStencilView(
		shadowMap,
		&depthStencilViewDesc,
		&shadowDepthView
	);

	res = game->device->CreateShaderResourceView(
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
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

	// Point filtered shadows can be faster, and may be a good choice when
	// rendering on hardware with lower feature levels. This sample has a
	// UI option to enable/disable filtering so you can see the difference
	// in quality and speed.

	game->device->CreateSamplerState(
		&comparisonSamplerDesc,
		&comparisonSampler
	);

	D3D11_RASTERIZER_DESC shadowRenderStateDesc;
	ZeroMemory(&shadowRenderStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	shadowRenderStateDesc.CullMode = D3D11_CULL_FRONT;
	shadowRenderStateDesc.FillMode = D3D11_FILL_SOLID;
	shadowRenderStateDesc.DepthClipEnable = true;

	res = game->device->CreateRasterizerState(&shadowRenderStateDesc, &shadowRastState);


	D3D11_BUFFER_DESC viewProjectionConstantBufferDesc = {};
	viewProjectionConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewProjectionConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewProjectionConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewProjectionConstantBufferDesc.MiscFlags = 0;
	viewProjectionConstantBufferDesc.StructureByteStride = 0;
	viewProjectionConstantBufferDesc.ByteWidth = sizeof(LightViewProjection);

	game->device->CreateBuffer(
		&viewProjectionConstantBufferDesc,
		nullptr,
		&constantLightViewProjectionBuffer
	);

	// Init viewport for shadow rendering
	ZeroMemory(&shadowViewport, sizeof(D3D11_VIEWPORT));
	shadowViewport.Height = shadowMapWidth;
	shadowViewport.Width = shadowMapHeight;
	shadowViewport.MinDepth = 0.f;
	shadowViewport.MaxDepth = 1.f;
}

GAMEFRAMEWORK_API std::vector<Vector4> DirectionalLight::GetFrustumCornersWorldSpace(const Matrix& view, const Matrix& proj)
{
	const auto viewProj = view * proj;
	const auto inv = viewProj.Invert();

	std::vector<Vector4> frustumCorners;
	frustumCorners.reserve(8);

	for (unsigned int z = 0; z < 2; ++z)
	{
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				const Vector4 pt =
					Vector4::Transform(Vector4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						z,
						1.0f), inv);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

GAMEFRAMEWORK_API void DirectionalLight::UpdateViewProjection()
{
	float maxCascadeDistance = renderCamera->farZ - renderCamera->nearZ;
	float cascadeLength = maxCascadeDistance / ((float)GlobalSettings::CASCADES_COUNT);
	//int currentShadowMapWidth = shadowMapWidth;
	//float cameraWidthMultiplier = 0.5f;
	
	Matrix view = renderCamera->GetViewMatrix();
	Matrix projection = renderCamera->GetProjectionMatrix();

	std::vector<Vector4> corners = GetFrustumCornersWorldSpace(view, projection);

	for (int i = 0; i < GlobalSettings::CASCADES_COUNT; ++i)
	{
		float currentCascadeDistance = (i + 1) * cascadeLength;
		Vector3 cameraForward = Vector3::Transform(Vector3::Forward, renderCamera->rotation);
		Vector3 center = Vector3::Zero;

		std::vector<Vector4> cornersCopy = corners;
		for (int cornerIdx = 4; cornerIdx < 8; ++cornerIdx)
		{
			Vector3 dir = corners[cornerIdx - 4] - renderCamera->position;
			dir.Normalize();

			cornersCopy[cornerIdx] = corners[cornerIdx - 4] + currentCascadeDistance * dir;
			cornersCopy[cornerIdx - 4] = corners[cornerIdx - 4] + (currentCascadeDistance - cascadeLength) * dir;
		}

		for (const Vector4& v : cornersCopy)
		{
			center += {v.x, v.y, v.z};
		}
		center /= cornersCopy.size();

		/*
		Camera* lightCamera = new Camera(nearPlane, nearPlane + currentFarPlane, 90.0f, currentShadowMapWidth * cameraWidthMultiplier, currentShadowMapWidth * cameraWidthMultiplier * 5);
		lightCamera->SetOrthographic(true);

		CameraController camController = CameraController();
		camController.SetCamera(lightCamera);

		Vector3 camRight = Vector3::Transform(Vector3::Right, lightCamera->rotation);
		float pitchRadians = XMConvertToRadians(pitchDegree);
		camController.Rotate(camRight, pitchRadians);

		float yawRadians = XMConvertToRadians(yawDegree);
		camController.Rotate(Vector3::Up, yawRadians);

		Vector3 cameraForward = Vector3::Transform(Vector3::Forward, lightCamera->rotation);
		lightCamera->position = -cameraForward * (nearPlane)-Vector3::Forward * 500.0f;
		this->direction = { cameraForward.x, cameraForward.y, cameraForward.z, 1.0f };

		viewProjection.view[i] = lightCamera->GetViewMatrix();
		viewProjection.projection[i] = lightCamera->GetProjectionMatrix();*/


		Matrix lightView = XMMatrixLookAtLH(center, center + this->direction, Vector3::Up);

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();

		for (const Vector4& v : cornersCopy)
		{
			Vector4 trf = Vector4::Transform(v, lightView);
			trf /= trf.w;

			minX = std::min(minX, trf.x);
			maxX = std::max(maxX, trf.x);
			minY = std::min(minY, trf.y);
			maxY = std::max(maxY, trf.y);
			minZ = std::min(minZ, trf.z);
			maxZ = std::max(maxZ, trf.z);
		}

		constexpr float zMult = 10.0f;

		minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
		maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

		viewProjection.viewProjection[i] = lightView * XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);

		viewProjection.distances[i] = renderCamera->farZ / (GlobalSettings::CASCADES_COUNT - i);
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	game->context->Map(constantLightViewProjectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &viewProjection, sizeof(viewProjection));
	game->context->Unmap(constantLightViewProjectionBuffer, 0);
}

GAMEFRAMEWORK_API void DirectionalLight::Release()
{
	shadowMap->Release();
	shadowDepthView->Release();
	shadowResourceView->Release();
	shadowRastState->Release();
	constantLightViewProjectionBuffer->Release();
}
