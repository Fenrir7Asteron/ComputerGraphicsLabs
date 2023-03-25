#pragma once
#include "CameraController.h"
#include "InputDevice.h"
#include "DisplayWin.h"
#include "GameComponent.h"

class OrbitalCameraController : public CameraController
{
public:
	GAMEFRAMEWORK_API OrbitalCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float maxCameraMoveSpeed, float cameraCloseupSpeed,
		GameComponent* attachedToBody, float distanceToBodySurface, float minDistanceToBodySurface, float maxDistanceToBodySurface);

	GAMEFRAMEWORK_API virtual void MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData);
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	void SetTargetBody(GameComponent* targetBody);

	InputDevice* inputDevice;
	DisplayWin* displayWin;
	float mouseSensitivity;
	float maxCameraMoveSpeedDuringTransition;
	float cameraCloseupSpeed;
	GameComponent* attachedToBody;
	float distanceToBodySurface;
	float minDistanceToBodySurface;
	float maxDistanceToBodySurface;
	bool isTransitioning;
};

