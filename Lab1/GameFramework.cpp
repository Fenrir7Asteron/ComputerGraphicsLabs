#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "GameFramework.h"
#include "FPSCameraController.h"
#include "DebugRenderSysImpl.h"
#include "windowsx.h"
#include <iostream>
#include <d3dcompiler.h>
#include "LightConstantData.h"
#include "PointLight.h"
#include "MacrosParser.h"

using namespace DirectX::SimpleMath;

GameFramework* GameFramework::Instance = nullptr;

GameFramework::GameFramework(LPCWSTR applicationName)
{
	GameFramework::Instance = this;

	this->applicationName = applicationName;
}

void GameFramework::Init(int screenWidth, int screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenWidth;

	displayWin = new DisplayWin();
	displayWin->CreateGameWindow(applicationName, screenWidth, screenHeight);

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

	auto res = D3D11CreateDeviceAndSwapChain(
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

	D3D11_DEPTH_STENCIL_DESC dsDescLess = {};
	dsDescLess.DepthEnable = TRUE;
	dsDescLess.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDescLess.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&dsDescLess, &depthStateLightingLess);

	D3D11_DEPTH_STENCIL_DESC dsDescGreater = {};
	dsDescGreater.DepthEnable = TRUE;
	dsDescGreater.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDescGreater.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	device->CreateDepthStencilState(&dsDescGreater, &depthStateLightingGreater);

	// create depth stencil texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
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

	viewport = {};
	viewport.Width = static_cast<float>(screenWidth);
	viewport.Height = static_cast<float>(screenHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

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


	D3D11_BLEND_DESC BlendStateLightDesc;
	ZeroMemory(&BlendStateLightDesc, sizeof(D3D11_BLEND_DESC));
	BlendStateLightDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendStateLightDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	BlendStateLightDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	BlendStateLightDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	BlendStateLightDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendStateLightDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	BlendStateLightDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendStateLightDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendStateLightDesc.IndependentBlendEnable = false;
	device->CreateBlendState(&BlendStateLightDesc, &blendStateLight);

	res = device->CreateShaderResourceView(
		gBuffer.normalMap,
		&normalSRVDesc,
		&gBuffer.normalSRV
	);

	inputDevice = new InputDevice(this);

	camera = new Camera(20.0f, 10000.0f, 50.0f, screenWidth, screenHeight);

	cameraControllers.emplace_back(new FPSCameraController(inputDevice, displayWin, 0.005f, 20000.0f));

	for (CameraController* controller : cameraControllers)
	{
		controller->SetCamera(camera);
	}

	debugRender = new DebugRenderSysImpl(this);
	debugRender->SetCamera(camera);


	// Create lighting pass resources shaders
	ID3DBlob* vertexBC = nullptr;
	ID3DBlob* errorVertexCode = nullptr;

	const LPCWSTR lightingPassShaderPath = L"./Shaders/LightingShader.hlsl";

	for (auto flag : vsLightingFlags)
	{
		auto macros = MacrosParser::GetMacros(flag);

		res = D3DCompileFromFile(lightingPassShaderPath,
			&macros[0],
			nullptr /*include*/,
			"VSMain",
			"vs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&vertexBC,
			&errorVertexCode);

		CheckShaderCreationSuccess(res, errorVertexCode, lightingPassShaderPath);

		res = device->CreateVertexShader(
			vertexBC->GetBufferPointer(),
			vertexBC->GetBufferSize(),
			nullptr, &lightingVertexShaders[flag]);
	}

	ID3DBlob* pixelBC = nullptr;
	ID3DBlob* errorPixelCode = nullptr;


	for (auto flag : psLightingFlags)
	{
		auto macros = MacrosParser::GetMacros(flag);

		res = D3DCompileFromFile(lightingPassShaderPath,
			&macros[0] /*macros*/,
			nullptr /*include*/,
			"PSMain",
			"ps_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&pixelBC,
			&errorPixelCode);

		CheckShaderCreationSuccess(res, errorPixelCode, lightingPassShaderPath);

		res = device->CreatePixelShader(
			pixelBC->GetBufferPointer(),
			pixelBC->GetBufferSize(),
			nullptr, &lightingPixelShaders[flag]);
	}

	if (FAILED(res))
	{
		return;
	}

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
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
		inputElements,
		1,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&lightingLayout);


	D3D11_BUFFER_DESC constantLightBufDesc = {};
	constantLightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantLightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantLightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantLightBufDesc.MiscFlags = 0;
	constantLightBufDesc.StructureByteStride = 0;
	constantLightBufDesc.ByteWidth = sizeof(LightConstantData);

	device->CreateBuffer(&constantLightBufDesc, nullptr, &constantLightBuffer);

	D3D11_BUFFER_DESC constantPointLightBufDesc = {};
	constantPointLightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantPointLightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantPointLightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantPointLightBufDesc.MiscFlags = 0;
	constantPointLightBufDesc.StructureByteStride = 0;
	constantPointLightBufDesc.ByteWidth = sizeof(PointLightConstantData);

	device->CreateBuffer(&constantPointLightBufDesc, nullptr, &constantPointLightBuffer);

	CD3D11_RASTERIZER_DESC rastCullBackDesc = {};
	rastCullBackDesc.CullMode = D3D11_CULL_BACK;
	rastCullBackDesc.FillMode = D3D11_FILL_SOLID;

	res = device->CreateRasterizerState(&rastCullBackDesc, &rastStateCullBack);

	CD3D11_RASTERIZER_DESC rastCullFrontDesc = {};
	rastCullFrontDesc.CullMode = D3D11_CULL_FRONT;
	rastCullFrontDesc.FillMode = D3D11_FILL_SOLID;

	res = device->CreateRasterizerState(&rastCullFrontDesc, &rastStateCullFront);
}

void GameFramework::Run()
{
	PrevTime = std::chrono::steady_clock::now();
	float totalTimeClamped = 0;
	unsigned int frameCount = 0;

	MSG msg = {};
	bool isExitRequested = false;

	while (!isExitRequested) {
		auto	curTime = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		totalTimeClamped += deltaTime;

		UpdateFrameCount(frameCount, totalTimeClamped);

		// Handle the windows messages.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// If windows signals to end the application then exit out.
			if (msg.message == WM_QUIT) {
				isExitRequested = true;
				break;
			}

			if (msg.message == WM_KEYDOWN) {
				Keys keyEnum = static_cast<Keys>(msg.wParam);
				inputDevice->AddPressedKey(keyEnum);
			}

			if (msg.message == WM_KEYUP) {
				Keys keyEnum = static_cast<Keys>(msg.wParam);
				inputDevice->RemovePressedKey(keyEnum);
			}

			if (msg.message == WM_MOUSEMOVE) {
				InputDevice::RawMouseEventArgs args;
				args.X = GET_X_LPARAM(msg.lParam);
				args.Y = GET_Y_LPARAM(msg.lParam);
				args.WheelDelta = 0;
				inputDevice->OnMouseMove(args);
				displayWin->CenterMouse();
			}

			if (msg.message == WM_MOUSEWHEEL) {
				InputDevice::RawMouseEventArgs args;
				args.X = 0;
				args.Y = 0;
				args.WheelDelta = (float) GET_WHEEL_DELTA_WPARAM(msg.wParam) / WHEEL_DELTA;
				inputDevice->OnMouseMove(args);
			}
		}

		if (isExitRequested)
			break;

		Update();

		RestoreTargets();

		RenderShadowMap();
		GeometryPass();
		LightingPass();
	}

	FreeGameResources();
}

void GameFramework::UpdateFrameCount(unsigned int &frameCount, float &totalTimeClamped)
{
	frameCount++;

	if (totalTimeClamped > 1.0f) {
		float fps = frameCount / totalTimeClamped;

		totalTimeClamped -= 1.0f;

		WCHAR text[256];
		swprintf_s(text, TEXT("FPS: %f"), fps);
		SetWindowText(displayWin->hWnd, text);

		frameCount = 0;
	}
}

void GameFramework::Update()
{
	for (auto gameComponent : gameComponents)
	{
		if (gameComponent->enabled)
			gameComponent->Update(deltaTime);
	}

	for (auto cameraController : cameraControllers)
	{
		cameraController->Update(deltaTime);
	}

	if (inputDevice->IsKeyDown(Keys::P))
	{
		camera->SetOrthographic(false);
	}

	if (inputDevice->IsKeyDown(Keys::O))
	{
		camera->SetOrthographic(true);
	}
}

void GameFramework::RenderShadowMap()
{
	context->ClearState();

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

	for (auto gameComponent : gameComponents)
	{
		if (gameComponent->castShadows)
			gameComponent->DrawShadowMap();
	}
}

GAMEFRAMEWORK_API void GameFramework::GeometryPass()
{
	context->ClearState();

	context->OMSetBlendState(blendStateOpaque, nullptr, 0xffffffff);

	context->OMSetDepthStencilState(depthStateGeometry.Get(), 0);
	ID3D11RenderTargetView* RTVs[] = { gBuffer.worldPos_DepthRTV, gBuffer.normalRTV, gBuffer.albedoRTV, gBuffer.diffuseCoeffRTV, gBuffer.specularCoeffRTV, gBuffer.ambientCoeffRTV };
	context->OMSetRenderTargets(6, RTVs, pDSV.Get());
	context->RSSetViewports(1, &viewport);

	for (auto gameComponent : gameComponents)
	{
		gameComponent->GeometryPass();
	}

	//debugRender->DrawGrid(20000.0f, 1000.0f, { 0.5f, 0.5f, 0.5f, 1.0f });
	//debugRender->Draw(deltaTime);

	//debugRender->Clear();
}

GAMEFRAMEWORK_API void GameFramework::LightingPass()
{
	context->ClearState();

	context->OMSetBlendState(blendStateLight, nullptr, 0xffffffff);
	context->RSSetState(rastStateCullBack);

	context->OMSetDepthStencilState(depthStateLightingLess.Get(), 0);
	context->OMSetRenderTargets(1, &rtv, pDSV.Get());

	ID3D11ShaderResourceView* SRVs[] = { gBuffer.worldPos_DepthSRV, gBuffer.normalSRV, gBuffer.albedoSRV, gBuffer.diffuseCoeffSRV, gBuffer.specularCoeffSRV, gBuffer.ambientCoeffSRV, dirLight->shadowResourceView };
	context->PSSetShaderResources(0, 7, SRVs);

	context->RSSetViewports(1, &viewport);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	LightConstantData lightData;
	lightData.cameraPos = Vector4(camera->position.x, camera->position.y, camera->position.z, 1.0f);
	lightData.lightParam1 = dirLight->direction;
	lightData.lightColor = dirLight->lightColor;
	lightData.DSAIntensity = Vector4(dirLight->diffuseIntensity, dirLight->specularIntensity, dirLight->ambientIntensity, 0.0f);
	lightData.CameraView = camera->GetViewMatrix();

	context->Map(constantLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &lightData, sizeof(lightData));
	context->Unmap(constantLightBuffer, 0);

	context->PSSetConstantBuffers(0, 1, &constantLightBuffer);
	context->PSSetConstantBuffers(1, 1, &dirLight->constantLightViewProjectionBuffer);

	context->PSSetSamplers(0, 1, dirLight->comparisonSampler.GetAddressOf());

	context->VSSetShader(lightingVertexShaders[VertexLightingShaderFlags::SCREEN_QUAD], nullptr, 0);
	context->PSSetShader(lightingPixelShaders[PixelLightingShaderFlags::DIRECTIONAL], nullptr, 0);
	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	context->Draw(4, 0);

	for (PointLight* pointLight : pointLights)
	{
		context->ClearState();

		context->OMSetBlendState(blendStateLight, nullptr, 0xffffffff);

		context->RSSetState(rastStateCullFront);

		ID3D11ShaderResourceView* SRVsPoint[] = { gBuffer.worldPos_DepthSRV, gBuffer.normalSRV, gBuffer.albedoSRV, gBuffer.diffuseCoeffSRV, gBuffer.specularCoeffSRV };
		context->PSSetShaderResources(0, 5, SRVsPoint);

		context->OMSetRenderTargets(1, &rtv, nullptr);

		context->OMSetDepthStencilState(depthStateLightingGreater.Get(), 0);

		context->RSSetViewports(1, &viewport);

		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		LightConstantData lightData;
		lightData.cameraPos = Vector4(camera->position.x, camera->position.y, camera->position.z, 1.0f);
		lightData.lightParam1 = Vector4(pointLight->lightPos.x, pointLight->lightPos.y, pointLight->lightPos.z, pointLight->range);
		lightData.lightColor = pointLight->lightColor;
		lightData.DSAIntensity = Vector4(pointLight->diffuseIntensity, pointLight->specularIntensity, 0.0f, 0.0f);
		lightData.CameraView = camera->GetViewMatrix();

		context->Map(constantLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &lightData, sizeof(lightData));
		context->Unmap(constantLightBuffer, 0);

		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		
		pointLight->pointLightMvp.worldMatrix = pointLight->lightMesh->GetWorldMatrix();
		pointLight->pointLightMvp.viewMatrix = camera->GetViewMatrix();
		pointLight->pointLightMvp.projectionMatrix = camera->GetProjectionMatrix();
		

		context->Map(constantPointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &pointLight->pointLightMvp, sizeof(pointLight->pointLightMvp));
		context->Unmap(constantPointLightBuffer, 0);

		context->PSSetConstantBuffers(0, 1, &constantLightBuffer);
		context->VSSetConstantBuffers(2, 1, &constantPointLightBuffer);

		context->IASetInputLayout(lightingLayout);
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT strides[] = { sizeof(Vertex) };
		UINT offsets[] = { 0 };

		context->IASetIndexBuffer(pointLight->lightMesh->meshPtrs[0].get()->ib, DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, &pointLight->lightMesh->meshPtrs[0].get()->vb, strides, offsets);

		context->VSSetShader(lightingVertexShaders[VertexLightingShaderFlags::NONE], nullptr, 0);
		context->PSSetShader(lightingPixelShaders[PixelLightingShaderFlags::POINT], nullptr, 0);

		context->DrawIndexed(pointLight->lightMesh->meshPtrs[0].get()->indicesLen, 0, 0);

		debugRender->DrawSphere(2.0f, pointLight->lightColor, Matrix::CreateTranslation({ pointLight->lightPos.x, pointLight->lightPos.y, pointLight->lightPos.z }), 16);
		//debugRender->DrawSphere(pointLight->range, { 0.0f, 1.0f, 1.0f, 1.0f }, Matrix::CreateTranslation({ pointLight->lightPos.x, pointLight->lightPos.y, pointLight->lightPos.z }), 16);
	}

	debugRender->DrawGrid(20000.0f, 1000.0f, { 0.5f, 0.5f, 0.5f, 1.0f });
	debugRender->Draw(deltaTime);

	context->OMSetRenderTargets(0, nullptr, nullptr);


	swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
	debugRender->Clear();
}

GAMEFRAMEWORK_API void GameFramework::AddComponent(GameComponent* gameComponent)
{
	gameComponents.push_back(gameComponent);
}

template<> GAMEFRAMEWORK_API void GameFramework::AddComponent(Model<BoundingOrientedBox>* gameComponent)
{
	gameComponents.push_back(gameComponent);
	physicalBoxComponents.emplace_back(&(gameComponent->physicalComponent));
}

template<> GAMEFRAMEWORK_API void GameFramework::AddComponent(Model<BoundingSphere>* gameComponent)
{
	gameComponents.push_back(gameComponent);
	physicalSphereComponents.emplace_back(&(gameComponent->physicalComponent));
}

template<typename T>
GAMEFRAMEWORK_API GameComponent* GameFramework::Intersects(PhysicalComponent<T>* queryingComponent)
{
	for (auto otherBoxComponent : physicalBoxComponents) {
		if (otherBoxComponent->parent->enabled == false || otherBoxComponent->parent == queryingComponent->parent)
			continue;

		if (queryingComponent->boundingShape.Intersects(otherBoxComponent->boundingShape)) {
			return otherBoxComponent->parent;
		}
	}

	for (auto otherSphere: physicalSphereComponents) {
		if (otherSphere->parent->enabled == false || otherSphere->parent == queryingComponent->parent)
			continue;

		if (queryingComponent->boundingShape.Intersects(otherSphere->boundingShape)) {
			return otherSphere->parent;
		}
	}

	return nullptr;
}

template GAMEFRAMEWORK_API GameComponent* GameFramework::Intersects(PhysicalComponent<BoundingSphere>* queryingComponent);
template GAMEFRAMEWORK_API GameComponent* GameFramework::Intersects(PhysicalComponent<BoundingOrientedBox>* queryingComponent);

//GAMEFRAMEWORK_API GameComponent* GameFramework::RayIntersectsSomething(PhysicalBoxComponent* queryingBox, DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 currentSpeed)
//{
//	for (auto otherBox : physicalBoxComponents) {
//		if (otherBox == queryingBox)
//			continue;
//
//		float minDistance = std::sqrt(currentSpeed.Length()) * 0.025f;
//		DirectX::SimpleMath::Vector3 direction = currentSpeed / currentSpeed.Length();
//		float intersectionDistance = 0.0f;
//
//		bool intersects = otherBox->boundingBox.Intersects(origin, direction, intersectionDistance);
//		if (intersects && intersectionDistance < minDistance) {
//			return otherBox;
//		}
//	}
//
//	return nullptr;
//}

void GameFramework::FreeGameResources()
{
	device.Reset();
	context->Release();
	swapChain->Release();
	backTex->Release();
	rtv->Release();

	for (auto gameComponent : gameComponents)
	{
		delete gameComponent;
	}

	gameComponents.clear();
	physicalBoxComponents.clear();
	physicalSphereComponents.clear();

	delete displayWin;
	delete inputDevice;

	delete camera;
	for (auto controller : cameraControllers)
	{
		delete controller;
	}
	cameraControllers.clear();

	dirLight->Release();
	delete dirLight;
}

GAMEFRAMEWORK_API void GameFramework::SetCameraController(int cameraIdx)
{
	if (cameraIdx < 0 || cameraIdx >= cameraControllers.size())
		return;

	for (int i = 0; i < cameraControllers.size(); ++i)
	{
		if (i != cameraIdx)
			cameraControllers[i]->camera = nullptr;
		else
			cameraControllers[i]->camera = camera;
	}
}

GAMEFRAMEWORK_API void GameFramework::RestoreTargets()
{
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(rtv, color);
	gBuffer.Clear(context, color);
	context->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->ClearDepthStencilView(dirLight->shadowDepthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

GAMEFRAMEWORK_API void GameFramework::CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName)
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