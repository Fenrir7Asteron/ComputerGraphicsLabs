#pragma once
#include "CameraController.h"
#include "InputDevice.h"

class DisplayWin;

class FPSCameraController : public CameraController
{
public:
	GAMEFRAMEWORK_API FPSCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float cameraSpeed);
	GAMEFRAMEWORK_API virtual void MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData);

	// Inherited via CameraController
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;

	InputDevice* inputDevice;
	DisplayWin* displayWin;
	float mouseSensitivity;
	float cameraSpeed;
};

