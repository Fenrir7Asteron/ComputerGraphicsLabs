//#include "pch.h"
#include "InputDevice.h"
#include <iostream>
#include "DisplayWin.h"


using namespace DirectX::SimpleMath;


GAMEFRAMEWORK_API InputDevice::InputDevice()
{
	keys = new std::unordered_set<Keys>();
}

GAMEFRAMEWORK_API InputDevice::~InputDevice()
{
	delete keys;
}

GAMEFRAMEWORK_API void InputDevice::OnKeyDown(KeyboardInputEventArgs args)
{
	bool Break = args.Flags & 0x01;

	auto key = static_cast<Keys>(args.VKey);

	if (args.MakeCode == 42) key = Keys::LeftShift;
	if (args.MakeCode == 54) key = Keys::RightShift;
	
	if(Break) {
		if(keys->count(key))	RemovePressedKey(key);
	} else {
		if (!keys->count(key))	AddPressedKey(key);
	}
}

GAMEFRAMEWORK_API void InputDevice::OnMouseMove(RawMouseEventArgs args)
{
	if(args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonDown))
		AddPressedKey(Keys::LeftButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonUp))
		RemovePressedKey(Keys::LeftButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonDown))
		AddPressedKey(Keys::RightButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonUp))
		RemovePressedKey(Keys::RightButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonDown))
		AddPressedKey(Keys::MiddleButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonUp))
		RemovePressedKey(Keys::MiddleButton);

	MouseOffset		= Vector2(args.X, args.Y);
	MouseWheelDelta = args.WheelDelta;

	const MouseMoveEventArgs moveArgs = {MouseOffset, MouseWheelDelta};
	
	MouseMove.Broadcast(moveArgs);
}

GAMEFRAMEWORK_API void InputDevice::AddPressedKey(Keys key)
{
	keys->insert(key);
}

GAMEFRAMEWORK_API void InputDevice::RemovePressedKey(Keys key)
{
	keys->erase(key);
}

GAMEFRAMEWORK_API bool InputDevice::IsKeyDown(Keys key)
{
	return keys->count(key);
}

