#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

#include "GameFramework.h"
#include "Camera/Camera.h"
#include "Camera/FPSCameraController.h"
#include "Render/Debug/DebugRenderSysImpl.h"
#include "Render/RenderAPI/RenderAPI.h"
#include "Render/RenderAPI/DirectX11/DirectX11RenderAPI.h"
#include "Component/GameObject.h"
#include "Component/Render/Light/PointLight.h"
#include "Component/Render/Light/LightConstantData.h"
#include "Component/Physics/PhysicalBoxComponent.h"
#include "Component/Physics/PhysicalSphereComponent.h"
#include "windowsx.h"
#include <iostream>
#include <d3dcompiler.h>
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

	m_renderer = new RenderAPI::DirectX11::DirectX11RenderAPI(screenWidth, screenHeight, displayWin);

	inputDevice = new InputDevice(this);

	camera = new Camera(0.1f, 10000.0f, 50.0f, screenWidth, screenHeight);
	m_renderer->SetCamera(camera);

	cameraControllers.push_back(new FPSCameraController(inputDevice, displayWin, 0.005f, 20000.0f));

	for (CameraController* controller : cameraControllers)
	{
		controller->SetCamera(camera);
	}
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
		m_renderer->Render(deltaTime);
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

GameObject* GameFramework::CreateGameObject(std::string name)
{
	auto* gameObject = new GameObject(name);
	gameObjects.push_back(gameObject);
	return gameObject;
}

GAMEFRAMEWORK_API void GameFramework::AddComponent(GameComponent* gameComponent)
{
	AddComponentInternal(gameComponent);
}

GAMEFRAMEWORK_API void GameFramework::AddComponentInternal(GameComponent* gameComponent)
{
	gameComponents.push_back(gameComponent);

	auto* renderComponent = dynamic_cast<RenderComponent*>(gameComponent);
	if (renderComponent != nullptr)
		m_renderer->renderComponents.push_back(renderComponent);

	auto* boxComponent = dynamic_cast<PhysicalBoxComponent*>(gameComponent);
	if (boxComponent != nullptr)
		physicalBoxComponents.push_back(boxComponent);

	auto* sphereComponent = dynamic_cast<PhysicalSphereComponent*>(gameComponent);
	if (sphereComponent != nullptr)
		physicalSphereComponents.push_back(sphereComponent);
}

GAMEFRAMEWORK_API void GameFramework::AddParticleSystem(ParticleSystemSorted* particleSystem)
{
	m_renderer->particleSystems.push_back(particleSystem);
}

GAMEFRAMEWORK_API PhysicalComponent* GameFramework::Intersects(PhysicalComponent* queryingComponent)
{
	for (auto otherBoxComponent : physicalBoxComponents) {
		if (queryingComponent->Intersects(otherBoxComponent)) {
			return otherBoxComponent;
		}
	}

	for (auto otherSphere: physicalSphereComponents) {
		if (queryingComponent->Intersects(otherSphere)) {
			return otherSphere;
		}
	}

	return nullptr;
}

void GameFramework::FreeGameResources()
{
	m_renderer->Release();

	for (auto* gameObject: gameObjects)
	{
		gameObject->Destroy();
	}

	gameObjects.clear();
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
