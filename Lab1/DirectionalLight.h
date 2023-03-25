#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"
#include <d3d11.h>
#include <wrl/client.h>

GAMEFRAMEWORK_API class DirectionalLight
{
public:
	GAMEFRAMEWORK_API DirectionalLight();

	GAMEFRAMEWORK_API DirectionalLight(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		DirectX::SimpleMath::Vector4 direction,
		DirectX::SimpleMath::Vector4 lightColor,
		float diffuseIntensity, float specularIntensity, float ambientIntensity,
		float nearPlane = 100.0f, float farPlane = 10000000.0f,
		int shadowMapWidth = 512, int shadowMapHeight = 512);

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
	DirectX::SimpleMath::Matrix viewProjection;
};