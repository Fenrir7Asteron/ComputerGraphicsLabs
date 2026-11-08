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
#include "Render/RenderAPI/Vulkan/VulkanRenderAPI.h"
#include "Component/GameObject.h"
#include "Component/Render/Light/PointLight.h"
#include "Component/Render/Light/LightConstantData.h"
#include "Component/Physics/PhysicalBoxComponent.h"
#include "Component/Physics/PhysicalSphereComponent.h"
#include "windowsx.h"
#include <iostream>
#include <d3dcompiler.h>
#include "MacrosParser.h"
#include "DisplayWin.h"
#include <GLFW/glfw3.h>

using namespace DirectX::SimpleMath;

GameFramework* GameFramework::Instance = nullptr;
DisplayWin* GameFramework::displayWin = nullptr;
InputDevice* GameFramework::inputDevice = nullptr;

GAMEFRAMEWORK_API GameFramework::GameFramework(const std::string& applicationName)
{
	GameFramework::Instance = this;

	this->applicationName = applicationName;
}

GAMEFRAMEWORK_API void GameFramework::Init(int screenWidth, int screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenWidth;

	displayWin = new DisplayWin(applicationName, screenWidth, screenHeight);

	/*m_renderer = new RenderAPI::DirectX11::DirectX11RenderAPI(screenWidth, screenHeight, displayWin);*/
	m_renderer = new RenderAPI::Vulkan::VulkanRenderAPI(screenWidth, screenHeight, displayWin);

	inputDevice = new InputDevice();

	camera = new Camera(0.1f, 10000.0f, 50.0f, screenWidth, screenHeight);
	m_renderer->SetCamera(camera);

	cameraControllers.push_back(new FPSCameraController(inputDevice, displayWin, 0.005f, 20000.0f));

	for (CameraController* controller : cameraControllers)
	{
		controller->SetCamera(camera);
	}
}

GAMEFRAMEWORK_API void GameFramework::Run()
{
	PrevTime = std::chrono::steady_clock::now();
	float totalTimeClamped = 0;
	unsigned int frameCount = 0;

	glfwSetKeyCallback(displayWin->window, KeyCallback);
	glfwSetCursorPosCallback(displayWin->window, CursorPosCallback);
	glfwSetMouseButtonCallback(displayWin->window, MouseButtonCallback);
	glfwSetScrollCallback(displayWin->window, ScrollCallback);

	// Main loop
	while (!glfwWindowShouldClose(displayWin->window)) {
		auto	curTime = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		totalTimeClamped += deltaTime;

		UpdateFrameCount(frameCount, totalTimeClamped);

		glfwPollEvents();

		Update();
		m_renderer->Render(deltaTime);
	}

	FreeGameResources();
}

GAMEFRAMEWORK_API void GameFramework::UpdateFrameCount(unsigned int &frameCount, float &totalTimeClamped)
{
	frameCount++;

	if (totalTimeClamped > 1.0f) {
		float fps = frameCount / totalTimeClamped;

		totalTimeClamped -= 1.0f;

		CHAR text[256];
		sprintf_s(text, "FPS: %f", fps);
		glfwSetWindowTitle(displayWin->window, text);

		frameCount = 0;
	}
}

GAMEFRAMEWORK_API void GameFramework::Update()
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

GAMEFRAMEWORK_API GameObject* GameFramework::CreateGameObject(std::string name)
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

GAMEFRAMEWORK_API void GameFramework::FreeGameResources()
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

GAMEFRAMEWORK_API void GameFramework::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (!displayWin->isCursorLocked)
		return;

	if (action == GLFW_PRESS) {
		std::cout << "Key: " << key << std::endl;

		Keys keyEnum = static_cast<Keys>(key);
		inputDevice->AddPressedKey(keyEnum);

		if (key == GLFW_KEY_ESCAPE)
		{
			displayWin->LockCursor(false);
		}
	}

	if (action == GLFW_RELEASE) {
		Keys keyEnum = static_cast<Keys>(key);
		inputDevice->RemovePressedKey(keyEnum);
	}
}

GAMEFRAMEWORK_API void GameFramework::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (!displayWin->isCursorLocked)
		return;

	InputDevice::RawMouseEventArgs args;
	args.X = xpos;
	args.Y = ypos;
	args.WheelDelta = 0;
	inputDevice->OnMouseMove(args);
	displayWin->CenterMouse();
}

GAMEFRAMEWORK_API void GameFramework::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (!displayWin->isCursorLocked)
		{
			displayWin->LockCursor(true);
		}
	}
}

GAMEFRAMEWORK_API void GameFramework::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (!displayWin->isCursorLocked)
		return;

	InputDevice::RawMouseEventArgs args;

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	args.X = width / 2;
	args.Y = height / 2;

	args.WheelDelta = (float) yoffset;
	inputDevice->OnMouseMove(args);
}
