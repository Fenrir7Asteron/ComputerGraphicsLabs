#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"
#include <d3d11.h>
#include <wrl/client.h>
#include "LightViewProjection.h"
#include "GlobalSettings.h"

GAMEFRAMEWORK_API class DirectionalLight
{
public:
	GAMEFRAMEWORK_API DirectionalLight();

	GAMEFRAMEWORK_API DirectionalLight(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		float pitchDegree,
		float yawDegree,
		DirectX::SimpleMath::Vector4 lightColor,
		float diffuseIntensity, float specularIntensity, float ambientIntensity,
		float farPlane = 1000000.0f,
		int shadowMapWidth = 2048, int shadowMapHeight = 2048);

	DirectX::SimpleMath::Vector4 direction;
	DirectX::SimpleMath::Vector4 lightColor;
	float diffuseIntensity;
	float specularIntensity;
	float ambientIntensity;

	ID3D11Texture2D* shadowMap;
	ID3D11DepthStencilView* shadowDepthView;
	ID3D11ShaderResourceView* shadowResourceView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> comparisonSampler;
	ID3D11RasterizerState* shadowRastState;
	ID3D11Buffer* constantLightViewProjectionBuffer;
	D3D11_VIEWPORT shadowViewport;
	LightViewProjection viewProjection;
};