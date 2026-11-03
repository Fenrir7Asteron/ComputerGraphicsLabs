#pragma once
#include "Camera/CameraController.h"
#include "InputDevice.h"

class DisplayWin;
class GameComponent;


class OrbitalCameraController : public CameraController
{
public:
	OrbitalCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float maxCameraMoveSpeed, float cameraCloseupSpeed,
		GameComponent* attachedToBody, float distanceToBodySurface, float minDistanceToBodySurface, float maxDistanceToBodySurface);

	virtual void MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData);
	virtual void Update(float deltaTime) override;
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

