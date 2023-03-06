#pragma once
#include "CameraController.h"
#include "InputDevice.h"
#include "DisplayWin.h"
#include "CelestialBody.h"

class OrbitalCameraController : public CameraController
{
public:
	GAMEFRAMEWORK_API OrbitalCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float maxCameraMoveSpeed, float cameraCloseupSpeed,
		CelestialBody* attachedToBody, float distanceToBodySurface, float maxDistanceToBodySurface);

	GAMEFRAMEWORK_API virtual void MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	void SetTargetBody(CelestialBody* targetBody);

	InputDevice* inputDevice;
	DisplayWin* displayWin;
	float mouseSensitivity;
	float maxCameraMoveSpeedDuringTransition;
	float cameraCloseupSpeed;
	CelestialBody* attachedToBody;
	float distanceToBodySurface;
	float maxDistanceToBodySurface;
	bool isTransitioning;

private:
	void SetIsTransitioning(bool isTransitioning);
};

