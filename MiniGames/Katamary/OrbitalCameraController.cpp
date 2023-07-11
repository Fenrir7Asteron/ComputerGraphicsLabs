#define NOMINMAX
#define _USE_MATH_DEFINES

#include "OrbitalCameraController.h"
#include <iostream>
#include <cmath>

using namespace DirectX::SimpleMath;

OrbitalCameraController::OrbitalCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float maxCameraMoveSpeedDuringTransition, float cameraCloseupSpeed,
	GameComponent* attachedToBody, float startDistanceToBodySurface, float minDistanceToBodySurface, float maxDistanceToBodySurface)
{
	inputDevice->MouseMove.AddRaw(this, &OrbitalCameraController::MouseMove);

	this->inputDevice = inputDevice;
	this->displayWin = displayWin;
	this->mouseSensitivity = mouseSensitivity;
	this->maxCameraMoveSpeedDuringTransition = maxCameraMoveSpeedDuringTransition;
	this->cameraCloseupSpeed = cameraCloseupSpeed;
	this->attachedToBody = attachedToBody;
	this->distanceToBodySurface = startDistanceToBodySurface;
	this->minDistanceToBodySurface = minDistanceToBodySurface;
	this->maxDistanceToBodySurface = maxDistanceToBodySurface;
}

void OrbitalCameraController::MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData)
{
	if (camera == nullptr || isTransitioning)
		return;

	Vector2 offsetFromCenter = displayWin->OffsetFromWindowCenter(mouseMoveData.Offset);

	if (std::abs(offsetFromCenter.x) > 0)
	{
		offsetFromCenter.x = std::max(-10.0f, offsetFromCenter.x);
		offsetFromCenter.x = std::min(10.0f, offsetFromCenter.x);
		Rotate(Vector3::Up, offsetFromCenter.x * mouseSensitivity);
	}

	if (std::abs(offsetFromCenter.y) > 0)
	{
		offsetFromCenter.y = std::max(-10.0f, offsetFromCenter.y);
		offsetFromCenter.y = std::min(10.0f, offsetFromCenter.y);
		Vector3 camRight = Vector3::Transform(Vector3::Right, camera->rotation);
		Rotate(camRight, offsetFromCenter.y * mouseSensitivity);
	}

	if (!isTransitioning && std::abs(mouseMoveData.WheelDelta) > 0)
	{
		distanceToBodySurface = std::min(distanceToBodySurface - (float) mouseMoveData.WheelDelta * cameraCloseupSpeed, this->maxDistanceToBodySurface);
		distanceToBodySurface = std::max(distanceToBodySurface, this->minDistanceToBodySurface);
	}
}

void OrbitalCameraController::Update(float deltaTime)
{
	if (camera == nullptr || attachedToBody == nullptr)
		return;

	Vector3 camForward = Vector3::Transform(Vector3::Forward, camera->rotation);

	Vector3 bodyPosition = attachedToBody->GetWorldMatrix().Translation();
	Vector3 cameraMoveTarget = bodyPosition - camForward * (distanceToBodySurface);
	Vector3 offsetToTarget = cameraMoveTarget - camera->position;

	float moveSpeed;
	float moveDistance;

	if (isTransitioning)
	{
		moveSpeed = this->maxCameraMoveSpeedDuringTransition;

		moveDistance = std::min(offsetToTarget.Length(), moveSpeed);

		Vector3 moveDirection = offsetToTarget;
		moveDirection.Normalize();

		Move(moveDirection * moveDistance);
	}
	else
	{
		Move(offsetToTarget);
	}
}

void OrbitalCameraController::SetTargetBody(GameComponent* targetBody)
{
	this->attachedToBody = targetBody;
}
