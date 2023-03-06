#pragma once
#include "CameraController.h"
#include "InputDevice.h"
#include "DisplayWin.h"

class FPSCameraController : public CameraController
{
public:
	GAMEFRAMEWORK_API FPSCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float cameraSpeed);
	GAMEFRAMEWORK_API virtual void MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;

	InputDevice* inputDevice;
	DisplayWin* displayWin;
	float mouseSensitivity;
	float cameraSpeed;
};

