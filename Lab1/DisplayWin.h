#pragma once

#include <windows.h>
#include <WinUser.h>

class DisplayWin
{
public:
	HWND CreateGameWindow(LPCWSTR applicationName, int windowWidth, int windowHeight);
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);
};
