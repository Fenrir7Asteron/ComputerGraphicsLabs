#pragma once
#include "CameraController.h"
#include "InputDevice.h"

class FPSCameraController : public CameraController
{
public:
	GAMEFRAMEWORK_API FPSCameraController(InputDevice* inputDevice);
	GAMEFRAMEWORK_API virtual void MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData);
};

