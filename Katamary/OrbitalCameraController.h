#pragma once
#include "CameraController.h"
#include "InputDevice.h"
#include "DisplayWin.h"
#include "KatamaryBall.h"

class OrbitalCameraController : public CameraController
{
public:
	GAMEFRAMEWORK_API OrbitalCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float maxCameraMoveSpeed, float cameraCloseupSpeed,
		KatamaryBall* attachedToBody, float distanceToBodySurface, float minDistanceToBodySurface, float maxDistanceToBodySurface);

	GAMEFRAMEWORK_API virtual void MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	void SetTargetBody(KatamaryBall* targetBody);

	InputDevice* inputDevice;
	DisplayWin* displayWin;
	float mouseSensitivity;
	float maxCameraMoveSpeedDuringTransition;
	float cameraCloseupSpeed;
	KatamaryBall* attachedToBody;
	float distanceToBodySurface;
	float minDistanceToBodySurface;
	float maxDistanceToBodySurface;
	bool isTransitioning;
};

