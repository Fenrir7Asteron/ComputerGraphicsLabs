#pragma once

#include <windows.h>
#include <WinUser.h>
#include "GameFrameworkExports.h"
#include "SimpleMath.h"

class DisplayWin
{
public:
	GAMEFRAMEWORK_API void CreateGameWindow(LPCWSTR applicationName, int windowWidth, int windowHeight);
	GAMEFRAMEWORK_API void SetMouseCapture(bool isMouseCaptured);
	GAMEFRAMEWORK_API void CenterMouse();
	GAMEFRAMEWORK_API DirectX::SimpleMath::Vector2 OffsetFromWindowCenter(DirectX::SimpleMath::Vector2 offset);

	HWND hWnd;
	int windowWidth;
	int windowHeight;
private:
	GAMEFRAMEWORK_API static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
};
