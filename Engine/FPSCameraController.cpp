#define NOMINMAX
#define _USE_MATH_DEFINES

#include "FPSCameraController.h"
#include <iostream>

using namespace DirectX::SimpleMath;

FPSCameraController::FPSCameraController(InputDevice* inputDevice, DisplayWin* displayWin, float mouseSensitivity, float cameraSpeed)
{
	inputDevice->MouseMove.AddRaw(this, &FPSCameraController::MouseMove);

	this->inputDevice = inputDevice;
	this->displayWin = displayWin;
	this->mouseSensitivity = mouseSensitivity;
	this->cameraSpeed = cameraSpeed;
}

GAMEFRAMEWORK_API void FPSCameraController::MouseMove(const InputDevice::MouseMoveEventArgs& mouseMoveData)
{
	if (camera == nullptr)
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
		offsetFromCenter.y = std::min(10.0f,  offsetFromCenter.y);
		Vector3 camRight = Vector3::Transform(Vector3::Right, camera->rotation);
		Rotate(camRight, offsetFromCenter.y * mouseSensitivity);
	}
}

GAMEFRAMEWORK_API void FPSCameraController::Update(float deltaTime)
{
	if (camera == nullptr)
		return;

	if (inputDevice->IsKeyDown(Keys::W))
	{
		Vector3 camForward = Vector3::Transform(Vector3::Forward, camera->rotation);
		Move(camForward * deltaTime * cameraSpeed);
	}

	if (inputDevice->IsKeyDown(Keys::S))
	{
		Vector3 camBackward = Vector3::Transform(Vector3::Backward, camera->rotation);
		Move(camBackward * deltaTime * cameraSpeed);
	}

	if (inputDevice->IsKeyDown(Keys::A))
	{
		Vector3 camLeft= Vector3::Transform(Vector3::Left, camera->rotation);
		Move(camLeft * deltaTime * cameraSpeed);
	}

	if (inputDevice->IsKeyDown(Keys::D))
	{
		Vector3 camRight = Vector3::Transform(Vector3::Right, camera->rotation);
		Move(camRight * deltaTime * cameraSpeed);
	}

	if (inputDevice->IsKeyDown(Keys::E))
	{
		Move(Vector3::Up * deltaTime * cameraSpeed);
	}

	if (inputDevice->IsKeyDown(Keys::Q))
	{
		Move(Vector3::Down * deltaTime * cameraSpeed);
	}
}
