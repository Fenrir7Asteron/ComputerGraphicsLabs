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
#include "DirectionalLight.h"
#include "Model.h"
#include "GBuffer.h"
#include "PointLight.h"

class DebugRenderSysImpl;

class GameFramework
{
public:
	GAMEFRAMEWORK_API GameFramework(LPCWSTR applicationName);
	GAMEFRAMEWORK_API virtual void Init(int windowWidth, int windowHeight);
	GAMEFRAMEWORK_API virtual void Run();
	GAMEFRAMEWORK_API virtual void UpdateFrameCount(unsigned int& frameCount, float& totalTimeClamped);
	GAMEFRAMEWORK_API virtual void Update();
	GAMEFRAMEWORK_API virtual void RenderShadowMap();
	GAMEFRAMEWORK_API virtual void GeometryPass();
	GAMEFRAMEWORK_API virtual void LightingPass();
	GAMEFRAMEWORK_API virtual void AddComponent(GameComponent* gameComponent);

	template <typename T>
	GAMEFRAMEWORK_API void AddComponent(Model<T>* gameComponent);

	template <typename T>
	GAMEFRAMEWORK_API GameComponent* Intersects(PhysicalComponent<T>* queryingObject);

	//GAMEFRAMEWORK_API virtual GameComponent* RayIntersectsSomething(PhysicalBoxComponent* queryingBox, DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 currentSpeed);
	
	GAMEFRAMEWORK_API virtual void FreeGameResources();
	GAMEFRAMEWORK_API virtual void SetCameraController(int cameraIdx);
	GAMEFRAMEWORK_API virtual void RestoreTargets();

	GAMEFRAMEWORK_API void CheckShaderCreationSuccess(const HRESULT res, ID3DBlob* errorVertexCode, const LPCWSTR shaderName);

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
	GBuffer gBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStateGeometry;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStateLightingLess;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStateLightingGreater;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
	D3D11_VIEWPORT viewport;

	ID3D11BlendState* blendStateOpaque;
	ID3D11BlendState* blendStateLight;

	std::vector<GameComponent*> gameComponents;
	std::vector<PhysicalComponent<BoundingOrientedBox>*> physicalBoxComponents;
	std::vector<PhysicalComponent<BoundingSphere>*> physicalSphereComponents;

	Camera* camera;
	std::vector<CameraController*> cameraControllers;

	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float deltaTime;

	DebugRenderSysImpl* debugRender;

	DirectionalLight* dirLight;
	std::vector<PointLight*> pointLights;

	ID3DBlob* directionalLightVertexBC;
	ID3D11VertexShader* directionalLightVertexShader;
	ID3DBlob* directionalLightPixelBC;
	ID3D11PixelShader* directionalLightPixelShader;
	ID3D11InputLayout* directionalLightLayout;

	ID3DBlob* pointLightVertexBC;
	ID3D11VertexShader* pointLightVertexShader;
	ID3DBlob* pointLightPixelBC;
	ID3D11PixelShader* pointLightPixelShader;
	ID3D11InputLayout* pointLightLayout;

	ID3D11Buffer* constantLightBuffer;

	ID3D11RasterizerState* rastStateCullBack;
	ID3D11RasterizerState* rastStateCullFront;
};

