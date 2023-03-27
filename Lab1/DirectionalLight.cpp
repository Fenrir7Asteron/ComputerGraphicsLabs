#define NOMINMAX
#define _USE_MATH_DEFINES

#include "DirectionalLight.h"
#include "Camera.h"
#include "CameraController.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API DirectionalLight::DirectionalLight()
{
}

GAMEFRAMEWORK_API DirectionalLight::DirectionalLight(
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	float pitchDegree,
	float yawDegree,
	Vector4 lightColor,
	float diffuseIntensity, float specularIntensity, float ambientIntensity, 
	float farPlane,
	int shadowMapWidth,	int shadowMapHeight)
{
	this->lightColor = lightColor;
	this->diffuseIntensity = diffuseIntensity;
	this->specularIntensity = specularIntensity;
	this->ambientIntensity = ambientIntensity;


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

	HRESULT res = device->CreateTexture2D(
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
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
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
	viewProjectionConstantBufferDesc.ByteWidth = sizeof(LightViewProjection);

	float cascadeLength = farPlane / ((float) GlobalSettings::CASCADES_COUNT);
	int currentShadowMapWidth = shadowMapWidth;
	
	for (int i = 0; i < GlobalSettings::CASCADES_COUNT; ++i)
	{
		float currentNearPlane = i * cascadeLength;
		float currentFarPlane = (i + 1) * cascadeLength;
		Camera* lightCamera = new Camera(currentNearPlane, currentFarPlane, 90.0f, shadowMapWidth, shadowMapHeight * 5);
		lightCamera->SetOrthographic(true);

		CameraController camController = CameraController();
		camController.SetCamera(lightCamera);

		Vector3 camRight = Vector3::Transform(Vector3::Right, lightCamera->rotation);
		float pitchRadians = XMConvertToRadians(pitchDegree);
		camController.Rotate(camRight, pitchRadians);

		float yawRadians = XMConvertToRadians(yawDegree);
		camController.Rotate(Vector3::Up, yawRadians);

		Vector3 cameraForward = Vector3::Transform(Vector3::Forward, lightCamera->rotation);
		lightCamera->position = -cameraForward * (currentFarPlane * 0.1f);
		this->direction = { cameraForward.x, cameraForward.y, cameraForward.z, 1.0f };

		viewProjection.view[i] = lightCamera->GetViewMatrix();
		viewProjection.projection[i] = lightCamera->GetProjectionMatrix();
		

		/*Matrix view = lightCamera->GetViewMatrix();
		Matrix projection = lightCamera->GetProjectionMatrix();
		
		std::vector<Vector4> corners = GetFrustumCornersWorldSpace(view, projection);
		Vector3 center = Vector3::Zero;
		for (const Vector4& v : corners)
		{
			center += {v.x, v.y, v.z};
		}
		center /= corners.size();

		viewProjection.view[i] = XMMatrixLookAtLH(center, center + cameraForward, Vector3::Up);

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();

		for (const Vector4& v : corners)
		{
			const Vector4 trf = Vector4::Transform(v, viewProjection.view[i]);

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

		viewProjection.projection[i] = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);*/
		
		viewProjection.distances[i] = currentFarPlane;

		delete lightCamera;
		currentShadowMapWidth /= 2;
	}
	

	D3D11_SUBRESOURCE_DATA viewProjectionConstantBufferData = {};
	viewProjectionConstantBufferData.pSysMem = &viewProjection;
	viewProjectionConstantBufferData.SysMemPitch = 0;
	viewProjectionConstantBufferData.SysMemSlicePitch = 0;

	device->CreateBuffer(
		&viewProjectionConstantBufferDesc,
		&viewProjectionConstantBufferData,
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

	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
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
