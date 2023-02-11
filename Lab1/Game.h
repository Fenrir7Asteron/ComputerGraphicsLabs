#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <chrono>
#include <vector>

#include "DisplayWin.h"
#include "GameComponent.h"

class Game
{
public:
	Game(LPCWSTR applicationName);
	void Init(int windowWidth, int windowHeight);
	void Run();
	void UpdateFrameCount(unsigned int& frameCount, float& totalTimeClamped);
	void Update();
	void Render(float& totalTimeClamped);
	void FreeGameResources();

	int screenWidth;
	int screenHeight;
	LPCWSTR applicationName;
	DisplayWin displayWin;
	HWND hWnd;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* backTex;
	ID3D11RenderTargetView* rtv;

	std::vector<GameComponent*> gameComponents;

	std::chrono::time_point<std::chrono::steady_clock> PrevTime;
	float deltaTime;
};

