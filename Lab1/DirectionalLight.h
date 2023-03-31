#pragma once
#include "SimpleMath.h"
#include "GameFrameworkExports.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include "LightViewProjection.h"
#include "GlobalSettings.h"

class GameFramework;

class DirectionalLight
{
public:
	GAMEFRAMEWORK_API DirectionalLight();

	GAMEFRAMEWORK_API DirectionalLight(
		GameFramework* game,
		DirectX::SimpleMath::Vector4 lightDir,
		DirectX::SimpleMath::Vector4 lightColor,
		float diffuseIntensity, float specularIntensity, float ambientIntensity,
		Camera* renderCamera,
		float shadowDistance = 10000.0f,
		int shadowMapWidth = 1024, int shadowMapHeight = 1024);

	GAMEFRAMEWORK_API std::vector<DirectX::SimpleMath::Vector4>
		GetFrustumCornersWorldSpace(
			const DirectX::SimpleMath::Matrix& view,
			const DirectX::SimpleMath::Matrix& proj);

	GAMEFRAMEWORK_API void UpdateViewProjection();
	GAMEFRAMEWORK_API void Release();

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

	Camera* renderCamera;
	GameFramework* game;
	float shadowDistance;
};