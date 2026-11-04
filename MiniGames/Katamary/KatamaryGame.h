#pragma once
#include "GameFramework.h"

class OrbitalCameraController;

class KatamaryGame : public GameFramework
{
public:
	KatamaryGame(const std::string& applicationName);
	void Init(int windowWidth, int windowHeight) override;

	GameObject* SpawnPlayerController(
		const std::string& name,
		const std::string& modelDir, const std::string& modelName,
		Material* material, const PhongCoefficients& coeff,
		float moveSpeed,
		const DirectX::SimpleMath::Vector3& position = Vector3::Zero,
		const DirectX::SimpleMath::Quaternion& rotation = Quaternion::Identity,
		const DirectX::SimpleMath::Vector3& scale = Vector3::One,
		float startScale = 1.0f);

	void SpawnPickableItem(
		const std::string& name, 
		const std::string& modelDir, const std::string& modelName, 
		Material* material, const PhongCoefficients& coeff,
		const DirectX::SimpleMath::Vector3& position = Vector3::Zero,
		const DirectX::SimpleMath::Quaternion& rotation = Quaternion::Identity,
		const DirectX::SimpleMath::Vector3& scale = Vector3::One,
		float startScale = 1.0f);

	void Update() override;

	OrbitalCameraController* orbitalCamController;
	ParticleSystemSorted* fountainPS;
};
