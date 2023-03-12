#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <chrono>
#include <vector>

#include "DisplayWin.h"
#include "PhysicalBoxComponent.h"
#include "PhysicalSphereComponent.h"
#include "InputDevice.h"
#include "CameraController.h"

class DebugRenderSysImpl;

class GameFramework
{
public:
	GAMEFRAMEWORK_API GameFramework(LPCWSTR applicationName);
	GAMEFRAMEWORK_API virtual void Init(int windowWidth, int windowHeight);
	GAMEFRAMEWORK_API virtual void Run();
	GAMEFRAMEWORK_API virtual void UpdateFrameCount(unsigned int& frameCount, float& totalTimeClamped);
	GAMEFRAMEWORK_API virtual void Update();
	GAMEFRAMEWORK_API virtual void Render(float& totalTimeClamped);
	GAMEFRAMEWORK_API virtual void AddComponent(GameComponent* gameComponent);
	GAMEFRAMEWORK_API virtual void AddComponent(PhysicalBoxComponent* gameComponent);
	GAMEFRAMEWORK_API virtual void AddComponent(PhysicalSphereComponent* gameComponent);
	GAMEFRAMEWORK_API virtual GameComponent* Intersects(PhysicalBoxComponent* queryingBox);
	GAMEFRAMEWORK_API virtual GameComponent* Intersects(PhysicalSphereComponent* queryingSphere);
	GAMEFRAMEWORK_API virtual GameComponent* RayIntersectsSomething(PhysicalBoxComponent* queryingBox, DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 currentSpeed);
	GAMEFRAMEWORK_API virtual void FreeGameResources();
	GAMEFRAMEWORK_API virtual void SetCameraController(int cameraIdx);
	GAMEFRAMEWORK_API virtual void RestoreTargets();

	GAMEFRAMEWORK_API static GameFramework* Instance;

	int screenWidth;
	int screenHeight;
	LPCWSTR applicationName;
	DisplayWin* displayWin;
	InputDevice* inputDevice;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* backTex;
	ID3D11RenderTargetView* rtv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;

	std::vector<GameComponent*> gameComponents;
	std::vector<PhysicalBoxComponent*> physicalBoxComponents;
	std::vector<PhysicalSphereComponent*> physicalSphereComponents;

	Camera* camera;
	std::vector<CameraController*> cameraControllers;

	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float deltaTime;

	DebugRenderSysImpl* debugRender;
};

