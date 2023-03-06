#define NOMINMAX
#define _USE_MATH_DEFINES

#include "OrbitalCameraController.h"
#include <iostream>
#include <cmath>

using namespace DirectX::SimpleMath;

OrbitalCameraController::OrbitalCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float maxCameraMoveSpeedDuringTransition, float cameraCloseupSpeed,
	CelestialBody* attachedToBody, float startDistanceToBodySurface, float maxDistanceToBodySurface)
{
	inputDevice->MouseMove.AddRaw(this, &OrbitalCameraController::MouseMove);

	this->inputDevice = inputDevice;
	this->displayWin = displayWin;
	this->mouseSensitivity = mouseSensitivity;
	this->maxCameraMoveSpeedDuringTransition = maxCameraMoveSpeedDuringTransition;
	this->cameraCloseupSpeed = cameraCloseupSpeed;
	this->attachedToBody = attachedToBody;
	this->distanceToBodySurface = startDistanceToBodySurface;
	this->maxDistanceToBodySurface = maxDistanceToBodySurface;

	SetIsTransitioning(false);
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
}

void OrbitalCameraController::Update(float deltaTime)
{
	if (camera == nullptr || attachedToBody == nullptr)
		return;

	Vector3 camForward = Vector3::Transform(Vector3::Forward, camera->rotation);

	Vector3 bodyPosition = attachedToBody->GetWorldMatrix().Translation();
	Vector3 cameraMoveTarget = bodyPosition - camForward * (attachedToBody->radius + distanceToBodySurface);
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

	if (isTransitioning)
		return;

	if (inputDevice->IsKeyDown(Keys::W))
	{
		distanceToBodySurface = std::max(distanceToBodySurface - deltaTime * cameraCloseupSpeed, 0.0f);
	}

	if (inputDevice->IsKeyDown(Keys::S))
	{
		distanceToBodySurface = std::min(distanceToBodySurface + deltaTime * cameraCloseupSpeed, this->maxDistanceToBodySurface);
	}
}

void OrbitalCameraController::SetTargetBody(CelestialBody* targetBody)
{
	this->attachedToBody = targetBody;
	SetIsTransitioning(false);
}

void OrbitalCameraController::SetIsTransitioning(bool isTransitioning)
{
	this->isTransitioning = isTransitioning;
}
