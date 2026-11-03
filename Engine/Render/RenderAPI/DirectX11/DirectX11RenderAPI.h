#pragma once
#include "GBuffer.h"
#include "Render/RenderAPI/RenderAPI.h"
#include <wrl.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct D3D11_VIEWPORT;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
class DebugRenderSysImpl;
class DisplayWin;

namespace RenderAPI::DirectX11
{
	class LightAPIComponent;

	class DirectX11RenderAPI : public RenderAPI
	{
	public:
		GAMEFRAMEWORK_API DirectX11RenderAPI(int screenWidth, int screenHeight, DisplayWin* displayWin);

		// Inherited via RenderAPI
		GAMEFRAMEWORK_API void Render(float deltaTime) override;
		GAMEFRAMEWORK_API void SetCamera(Camera* camera) override;
		GAMEFRAMEWORK_API void Release() override;

		GAMEFRAMEWORK_API virtual void RestoreTargets();
		GAMEFRAMEWORK_API virtual void RenderShadowMap();
		GAMEFRAMEWORK_API virtual void GeometryPass();
		GAMEFRAMEWORK_API virtual void LightingPass();
		GAMEFRAMEWORK_API virtual void ParticleSystemPass(float deltaTime);


		GAMEFRAMEWORK_API static DirectX11RenderAPI* Instance;
		LightAPIComponent* lightAPIComponent;

		Microsoft::WRL::ComPtr<ID3D11Device> device;
		ID3D11DeviceContext* context;
		IDXGISwapChain* swapChain;
		ID3D11Texture2D* backTex;
		ID3D11RenderTargetView* rtv;
		D3D11_VIEWPORT* viewport;

		GBuffer gBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStateGeometry;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;

		ID3D11BlendState* blendStateOpaque;

		ID3D11RasterizerState* rastStateCullBack;
		ID3D11RasterizerState* rastStateCullFront;

		DebugRenderSysImpl* debugRender;
	};
}
