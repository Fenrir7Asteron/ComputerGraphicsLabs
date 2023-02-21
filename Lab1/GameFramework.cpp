#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "GameFramework.h"
#include <iostream>

GameFramework::GameFramework(LPCWSTR applicationName)
{
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
	swapDesc.OutputWindow = *(displayWin->hWnd);
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

	inputDevice = new InputDevice(this);
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
		}

		if (isExitRequested)
			break;

		Update();
		Render(totalTimeClamped);
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
		SetWindowText(*(displayWin->hWnd), text);

		frameCount = 0;
	}
}

void GameFramework::Update()
{
	for (auto gameComponent : gameComponents)
	{
		gameComponent->Update(deltaTime);
	}
}

void GameFramework::Render(float& totalTimeClamped)
{

	context->ClearState();

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(screenWidth);
	viewport.Height = static_cast<float>(screenHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	context->RSSetViewports(1, &viewport);

	float intensity = totalTimeClamped;
	if (totalTimeClamped > 0.5f)
		intensity = 1.0f - totalTimeClamped;

	float color[] = { intensity * 0.5f, intensity * 0.5f, intensity * 0.5f + 0.25f, 1.0f };
	context->ClearRenderTargetView(rtv, color);

	for (auto gameComponent : gameComponents) 
	{
		gameComponent->Draw();
	}

	context->OMSetRenderTargets(1, &rtv, nullptr);

	context->OMSetRenderTargets(0, nullptr, nullptr);

	swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
}

GAMEFRAMEWORK_API void GameFramework::AddComponent(GameComponent* gameComponent)
{
	gameComponents.push_back(gameComponent);
}

GAMEFRAMEWORK_API void GameFramework::AddComponent(PhysicalBoxComponent* gameComponent)
{
	gameComponents.push_back(gameComponent);
	physicalGameComponents.push_back(gameComponent);
}

GAMEFRAMEWORK_API PhysicalBoxComponent* GameFramework::Intersects(PhysicalBoxComponent* queryingBox)
{
	for (auto otherBox : physicalGameComponents) {
		if (otherBox == queryingBox)
			continue;

		if (queryingBox->boundingBox.Intersects(otherBox->boundingBox)) {
			return otherBox;
		}
	}

	return nullptr;
}

GAMEFRAMEWORK_API PhysicalBoxComponent* GameFramework::RayIntersectsSomething(PhysicalBoxComponent* queryingBox, DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 direction)
{
	for (auto otherBox : physicalGameComponents) {
		if (otherBox == queryingBox)
			continue;

		direction.Normalize();
		float intersectionDistance = 0.0f;

		if (otherBox->boundingBox.Intersects(origin, direction, intersectionDistance) && intersectionDistance < 0.1f) {
			return otherBox;
		}
	}

	return nullptr;
}

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
	physicalGameComponents.clear();

	delete displayWin;
	delete inputDevice;
}
