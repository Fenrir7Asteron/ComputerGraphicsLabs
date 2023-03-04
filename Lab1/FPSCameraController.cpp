#include "FPSCameraController.h"
#include <iostream>

FPSCameraController::FPSCameraController(InputDevice* inputDevice)
{
	inputDevice->MouseMove.AddRaw(this, &FPSCameraController::MouseMove);
}

GAMEFRAMEWORK_API void FPSCameraController::MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData)
{
	std::cout << mouseMoveData.Offset.x << " " << mouseMoveData.Offset.y << std::endl;
	
}
