#pragma once

#include <windows.h>
#include <WinUser.h>
#include "GameFrameworkExports.h"

class DisplayWin
{
public:
	GAMEFRAMEWORK_API HWND CreateGameWindow(LPCWSTR applicationName, int windowWidth, int windowHeight);
private:
	GAMEFRAMEWORK_API static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
};
