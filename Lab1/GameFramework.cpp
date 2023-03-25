#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "GameFramework.h"
#include "FPSCameraController.h"
#include "DebugRenderSysImpl.h"
#include "windowsx.h"
#include <iostream>

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
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
	device->CreateDepthStencilState(&dsDesc, &pDSState);

	// bind depth state
	context->OMSetDepthStencilState(pDSState.Get(), 1);

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

	// bind depth stencil view
	context->OMSetRenderTargets(1, &rtv, pDSV.Get());

	inputDevice = new InputDevice(this);

	camera = new Camera(100.0f, 10000000.0f, 50.0f, screenWidth, screenHeight);

	cameraControllers.emplace_back(new FPSCameraController(inputDevice, displayWin, 0.005f, 20000.0f));

	for (CameraController* controller : cameraControllers)
	{
		controller->SetCamera(camera);
	}

	debugRender = new DebugRenderSysImpl(this);
	debugRender->SetCamera(camera);
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

	RestoreTargets();

	for (auto gameComponent : gameComponents) 
	{
		gameComponent->Draw();
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

	delete displayWin;
	delete inputDevice;

	delete camera;
	for (auto controller : cameraControllers)
	{
		delete controller;
	}
	cameraControllers.clear();
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
	float color[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	context->ClearRenderTargetView(rtv, color);
	context->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}
