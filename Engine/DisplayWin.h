#pragma once

#include "GameFrameworkExports.h"
#include "Utils/SimpleMath.h"
#include <string>

class GLFWwindow;

class DisplayWin
{
public:
	GAMEFRAMEWORK_API DisplayWin(const std::string& applicationName, int windowWidth, int windowHeight);
	GAMEFRAMEWORK_API ~DisplayWin();
	GAMEFRAMEWORK_API void SetMouseCapture(bool isMouseCaptured);
	GAMEFRAMEWORK_API void CenterMouse();
	GAMEFRAMEWORK_API DirectX::SimpleMath::Vector2 OffsetFromWindowCenter(DirectX::SimpleMath::Vector2 offset);
	GAMEFRAMEWORK_API void LockCursor(bool isLocked);

	GLFWwindow* window;
	HWND hWnd;
	int windowWidth;
	int windowHeight;
	bool isCursorLocked;
};
