#include <iostream>

#include "DisplayWin.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API DisplayWin::DisplayWin(const std::string& applicationName, int windowWidth, int windowHeight)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(windowWidth, windowHeight, applicationName.c_str(), nullptr, nullptr);
	glfwSwapInterval(1);

	this->hWnd = glfwGetWin32Window(window);

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	LockCursor(true);
	CenterMouse();

	this->windowWidth = windowWidth;
	this->windowHeight= windowHeight;
}

DisplayWin::~DisplayWin()
{
	glfwDestroyWindow(window);
}

GAMEFRAMEWORK_API void DisplayWin::SetMouseCapture(bool isMouseCaptured)
{
	std::cout << isMouseCaptured << std::endl;
	if (isMouseCaptured) 
	{
		SetCapture(hWnd);
	}
		
	else
		ReleaseCapture();
}

GAMEFRAMEWORK_API void DisplayWin::CenterMouse()
{
	POINT pt = { windowWidth / 2, windowHeight / 2 };
	ClientToScreen(hWnd, &pt);
	SetCursorPos(pt.x, pt.y);
}

GAMEFRAMEWORK_API Vector2 DisplayWin::OffsetFromWindowCenter(DirectX::SimpleMath::Vector2 offset)
{
	offset.x -= windowWidth / 2;
	offset.y -= windowHeight / 2;
	return offset;
}

GAMEFRAMEWORK_API void DisplayWin::LockCursor(bool isLocked)
{
	ShowCursor(!isLocked);
	SetMouseCapture(isLocked);

	isCursorLocked = isLocked;
}
