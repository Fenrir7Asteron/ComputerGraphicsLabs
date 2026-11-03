#include "DirectX11RenderAPI.h"
#include "LightAPIComponent.h"
#include "Render/Debug/DebugRenderSysImpl.h"
#include "Component/Render/RenderComponent.h"
#include "Component/Render/Light/DirectionalLight.h"
#include "Component/Render/Light/PointLight.h"
#include "Camera/Camera.h"
#include <d3dcommon.h>
#include <dxgi.h>
#include <d3d11.h>
#include <DisplayWin.h>
#include <Component/Render/Light/LightConstantData.h>
#include <ParticleSystemSorted.h>
#include <iostream>

namespace RenderAPI::DirectX11
{
	DirectX11RenderAPI* DirectX11RenderAPI::Instance = nullptr;

	GAMEFRAMEWORK_API DirectX11RenderAPI::DirectX11RenderAPI(int screenWidth, int screenHeight, DisplayWin* displayWin)
	{
		DirectX11RenderAPI::Instance = this;

		D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

		DXGI_SWAP_CHAIN_DESC swapDesc = {};
		swapDesc.BufferCount = 2;
		swapDesc.BufferDesc.Width = screenWidth;
		swapDesc.BufferDesc.Height = screenHeight;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.OutputWindow = displayWin->hWnd;
		swapDesc.Windowed = true;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;

		HRESULT res = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_DEBUG,
			featureLevel,
			1,
			D3D11_SDK_VERSION,
			&swapDesc,
			&swapChain,
			&device,
			nullptr,
			&context);

		if (FAILED(res))
		{
			// Well, that was unexpected
			return;
		}

		res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);	// __uuidof(ID3D11Texture2D)
		res = device->CreateRenderTargetView(backTex, nullptr, &rtv);


		// create depth stencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		device->CreateDepthStencilState(&dsDesc, &depthStateGeometry);

		// create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = screenWidth;
		descDepth.Height = screenHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

		// create view of depth stencil texture
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
		descDSV.Format = DXGI_FORMAT_D32_FLOAT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		device->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDSV);

		viewport = new D3D11_VIEWPORT();
		viewport->Width = static_cast<float>(screenWidth);
		viewport->Height = static_cast<float>(screenHeight);
		viewport->TopLeftX = 0;
		viewport->TopLeftY = 0;
		viewport->MinDepth = 0;
		viewport->MaxDepth = 1.0f;

		// Create GBuffer resources
		D3D11_TEXTURE2D_DESC diffuseMapDesc;
		ZeroMemory(&diffuseMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		diffuseMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		diffuseMapDesc.MipLevels = 1;
		diffuseMapDesc.ArraySize = 1;
		diffuseMapDesc.SampleDesc.Count = 1;
		diffuseMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		diffuseMapDesc.Height = static_cast<UINT>(screenWidth);
		diffuseMapDesc.Width = static_cast<UINT>(screenHeight);

		res = device->CreateTexture2D(
			&diffuseMapDesc,
			nullptr,
			&gBuffer.diffuseCoeffMap
		);

		D3D11_TEXTURE2D_DESC specularMapDesc;
		ZeroMemory(&specularMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		specularMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		specularMapDesc.MipLevels = 1;
		specularMapDesc.ArraySize = 1;
		specularMapDesc.SampleDesc.Count = 1;
		specularMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		specularMapDesc.Height = static_cast<UINT>(screenWidth);
		specularMapDesc.Width = static_cast<UINT>(screenHeight);

		res = device->CreateTexture2D(
			&specularMapDesc,
			nullptr,
			&gBuffer.specularCoeffMap
		);

		D3D11_TEXTURE2D_DESC ambientMapDesc;
		ZeroMemory(&ambientMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		ambientMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ambientMapDesc.MipLevels = 1;
		ambientMapDesc.ArraySize = 1;
		ambientMapDesc.SampleDesc.Count = 1;
		ambientMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		ambientMapDesc.Height = static_cast<UINT>(screenWidth);
		ambientMapDesc.Width = static_cast<UINT>(screenHeight);

		res = device->CreateTexture2D(
			&ambientMapDesc,
			nullptr,
			&gBuffer.ambientCoeffMap
		);

		D3D11_TEXTURE2D_DESC albedoMapDesc;
		ZeroMemory(&albedoMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		albedoMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		albedoMapDesc.MipLevels = 1;
		albedoMapDesc.ArraySize = 1;
		albedoMapDesc.SampleDesc.Count = 1;
		albedoMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		albedoMapDesc.Height = static_cast<UINT>(screenWidth);
		albedoMapDesc.Width = static_cast<UINT>(screenHeight);

		res = device->CreateTexture2D(
			&albedoMapDesc,
			nullptr,
			&gBuffer.albedoMap
		);

		D3D11_TEXTURE2D_DESC worldPosMapDesc;
		ZeroMemory(&worldPosMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		worldPosMapDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		worldPosMapDesc.MipLevels = 1;
		worldPosMapDesc.ArraySize = 1;
		worldPosMapDesc.SampleDesc.Count = 1;
		worldPosMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		worldPosMapDesc.Height = static_cast<UINT>(screenWidth);
		worldPosMapDesc.Width = static_cast<UINT>(screenHeight);

		res = device->CreateTexture2D(
			&worldPosMapDesc,
			nullptr,
			&gBuffer.worldPos_DepthMap
		);

		D3D11_TEXTURE2D_DESC normalMapDesc;
		ZeroMemory(&normalMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
		normalMapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		normalMapDesc.MipLevels = 1;
		normalMapDesc.ArraySize = 1;
		normalMapDesc.SampleDesc.Count = 1;
		normalMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		normalMapDesc.Height = static_cast<UINT>(screenWidth);
		normalMapDesc.Width = static_cast<UINT>(screenHeight);

		res = device->CreateTexture2D(
			&normalMapDesc,
			nullptr,
			&gBuffer.normalMap
		);

		res = device->CreateRenderTargetView(gBuffer.diffuseCoeffMap, nullptr, &gBuffer.diffuseCoeffRTV);
		res = device->CreateRenderTargetView(gBuffer.specularCoeffMap, nullptr, &gBuffer.specularCoeffRTV);
		res = device->CreateRenderTargetView(gBuffer.ambientCoeffMap, nullptr, &gBuffer.ambientCoeffRTV);
		res = device->CreateRenderTargetView(gBuffer.albedoMap, nullptr, &gBuffer.albedoRTV);
		res = device->CreateRenderTargetView(gBuffer.worldPos_DepthMap, nullptr, &gBuffer.worldPos_DepthRTV);
		res = device->CreateRenderTargetView(gBuffer.normalMap, nullptr, &gBuffer.normalRTV);

		D3D11_SHADER_RESOURCE_VIEW_DESC diffuseSRVDesc;
		ZeroMemory(&diffuseSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		diffuseSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		diffuseSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		diffuseSRVDesc.Texture2D.MipLevels = 1;
		diffuseSRVDesc.Texture2D.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(
			gBuffer.diffuseCoeffMap,
			&diffuseSRVDesc,
			&gBuffer.diffuseCoeffSRV
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC specularSRVDesc;
		ZeroMemory(&specularSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		specularSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		specularSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		specularSRVDesc.Texture2D.MipLevels = 1;
		specularSRVDesc.Texture2D.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(
			gBuffer.specularCoeffMap,
			&specularSRVDesc,
			&gBuffer.specularCoeffSRV
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC ambientSRVDesc;
		ZeroMemory(&ambientSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		ambientSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		ambientSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		ambientSRVDesc.Texture2D.MipLevels = 1;
		ambientSRVDesc.Texture2D.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(
			gBuffer.ambientCoeffMap,
			&ambientSRVDesc,
			&gBuffer.ambientCoeffSRV
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC albedoSRVDesc;
		ZeroMemory(&albedoSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		albedoSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		albedoSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		albedoSRVDesc.Texture2D.MipLevels = 1;
		albedoSRVDesc.Texture2D.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(
			gBuffer.albedoMap,
			&albedoSRVDesc,
			&gBuffer.albedoSRV
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC worldPosSRVDesc;
		ZeroMemory(&worldPosSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		worldPosSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		worldPosSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		worldPosSRVDesc.Texture2D.MipLevels = 1;
		worldPosSRVDesc.Texture2D.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(
			gBuffer.worldPos_DepthMap,
			&worldPosSRVDesc,
			&gBuffer.worldPos_DepthSRV
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC normalSRVDesc;
		ZeroMemory(&normalSRVDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		normalSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		normalSRVDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		normalSRVDesc.Texture2D.MipLevels = 1;
		normalSRVDesc.Texture2D.MostDetailedMip = 0;

		D3D11_BLEND_DESC BlendStateOpaqueDesc;
		ZeroMemory(&BlendStateOpaqueDesc, sizeof(D3D11_BLEND_DESC));
		BlendStateOpaqueDesc.RenderTarget[0].BlendEnable = FALSE;
		BlendStateOpaqueDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		BlendStateOpaqueDesc.IndependentBlendEnable = false;
		device->CreateBlendState(&BlendStateOpaqueDesc, &blendStateOpaque);


		res = device->CreateShaderResourceView(
			gBuffer.normalMap,
			&normalSRVDesc,
			&gBuffer.normalSRV
		);

		CD3D11_RASTERIZER_DESC rastCullBackDesc = {};
		rastCullBackDesc.CullMode = D3D11_CULL_BACK;
		rastCullBackDesc.FillMode = D3D11_FILL_SOLID;

		res = device->CreateRasterizerState(&rastCullBackDesc, &rastStateCullBack);

		CD3D11_RASTERIZER_DESC rastCullFrontDesc = {};
		rastCullFrontDesc.CullMode = D3D11_CULL_FRONT;
		rastCullFrontDesc.FillMode = D3D11_FILL_SOLID;

		res = device->CreateRasterizerState(&rastCullFrontDesc, &rastStateCullFront);


		debugRender = new DebugRenderSysImpl();
		lightAPIComponent = new LightAPIComponent();
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::Render(float deltaTime)
	{
		RestoreTargets();

		RenderShadowMap();
		GeometryPass();
		LightingPass();

		ParticleSystemPass(deltaTime);

		context->ClearState();
		context->OMSetRenderTargets(1, &rtv, pDSV.Get());

		debugRender->DrawGrid(20000.0f, 1000.0f, { 0.5f, 0.5f, 0.5f, 1.0f });
		debugRender->Draw(deltaTime);

		context->OMSetRenderTargets(0, nullptr, nullptr);

		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::SetCamera(Camera* camera)
	{
		RenderAPI::SetCamera(camera);
		debugRender->SetCamera(camera);
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::RestoreTargets()
	{
		float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->ClearRenderTargetView(rtv, color);
		gBuffer.Clear(context, color);
		context->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		context->ClearDepthStencilView(dirLight->shadowDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		debugRender->Clear();
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::RenderShadowMap()
	{
		context->ClearState();

		if (dirLight != nullptr && dirLight->enabled)
		{
			context->OMSetRenderTargets(
				0,
				nullptr,
				dirLight->shadowDepthView
			);

			context->RSSetState(dirLight->shadowRastState);
			context->OMSetDepthStencilState(depthStateGeometry.Get(), 1);
			context->RSSetViewports(1, &dirLight->shadowViewport);

			// Send the constant buffers to the Graphics device.
			context->GSSetConstantBuffers(
				0,
				1,
				&dirLight->constantLightViewProjectionBuffer
			);

			dirLight->UpdateViewProjection();
		}

		for (auto renderComponent : renderComponents)
		{
			if (renderComponent->castShadows)
				renderComponent->DrawShadowMap();
		}
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::GeometryPass()
	{
		context->ClearState();

		context->OMSetBlendState(blendStateOpaque, nullptr, 0xffffffff);

		context->OMSetDepthStencilState(depthStateGeometry.Get(), 0);
		ID3D11RenderTargetView* RTVs[] = { gBuffer.worldPos_DepthRTV, gBuffer.normalRTV, gBuffer.albedoRTV, gBuffer.diffuseCoeffRTV, gBuffer.specularCoeffRTV, gBuffer.ambientCoeffRTV };
		context->OMSetRenderTargets(6, RTVs, pDSV.Get());
		context->RSSetViewports(1, viewport);

		for (auto renderComponent : renderComponents)
		{
			renderComponent->GeometryPass();
		}

		//debugRender->DrawGrid(20000.0f, 1000.0f, { 0.5f, 0.5f, 0.5f, 1.0f });
		//debugRender->Draw(deltaTime);

		//debugRender->Clear();
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::LightingPass()
	{
		lightAPIComponent->DirectionalLightLightingPass(dirLight);
		lightAPIComponent->PointLightLightingPass(pointLights);
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::ParticleSystemPass(float deltaTime)
	{
		for (auto particleSystem : particleSystems)
		{
			particleSystem->Update(deltaTime);
		}

		for (auto particleSystem : particleSystems)
		{
			particleSystem->GeometryPass();
			std::cout << "Current particles total: " << particleSystem->ParticlesCount << ", dead: " << particleSystem->ParticlesDeadCount << std::endl;
		}
	}

	GAMEFRAMEWORK_API void DirectX11RenderAPI::Release()
	{
		device.Reset();
		context->Release();
		swapChain->Release();
		backTex->Release();
		rtv->Release();

		gBuffer.Release();

		depthStateGeometry->Release();
		pDepthStencil->Release();
		pDSV->Release();
		blendStateOpaque->Release();
		rastStateCullBack->Release();
		rastStateCullFront->Release();

		debugRender->Clear();

		delete viewport;
	}
}
