#include <iostream>
#include "DisplayWin.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API DisplayWin::DisplayWin(const std::string& applicationName, int windowWidth, int windowHeight)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(windowWidth, windowHeight, applicationName.c_str(), nullptr, nullptr);
	glfwSwapInterval(1);

	glfwShowWindow(window);

	LockCursor(true);
	CenterMouse();

	this->windowWidth = windowWidth;
	this->windowHeight= windowHeight;
}

DisplayWin::~DisplayWin()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

GAMEFRAMEWORK_API void DisplayWin::CenterMouse()
{
	glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

GAMEFRAMEWORK_API Vector2 DisplayWin::OffsetFromWindowCenter(DirectX::SimpleMath::Vector2 offset)
{
	offset.x -= windowWidth / 2;
	offset.y -= windowHeight / 2;
	return offset;
}

GAMEFRAMEWORK_API void DisplayWin::LockCursor(bool isLocked)
{
	if (isLocked)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	isCursorLocked = isLocked;
}
