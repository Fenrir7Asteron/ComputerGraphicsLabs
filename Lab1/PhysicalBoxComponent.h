#pragma once
#include "GameComponent.h"
#include "DirectXCollision.h"
#include "PhysicalLayer.h"

class GameFramework;

class PhysicalBoxComponent : public GameComponent
{
public:
	GAMEFRAMEWORK_API PhysicalBoxComponent(GameFramework* game,
		GameComponent* parent,
		DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero,
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::Identity,
		DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One,
		Material* material = nullptr);

	GAMEFRAMEWORK_API PhysicalBoxComponent(GameFramework* game,
		GameComponent* parent,
		PhysicalLayer physicalLayer = PhysicalLayer::Default,
		DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero,
		DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::Identity,
		DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One,
		Material* material = nullptr);

	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta) override;

	DirectX::BoundingBox boundingBox;
	PhysicalLayer physicalLayer = PhysicalLayer::Default;
};