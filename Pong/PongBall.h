#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3d11.h>
#include "PhysicalBoxComponent.h"
#include "Delegates.h"

class SimpleMath;
class PongRacket;
class PongGame;

class PongBall: public PhysicalBoxComponent
{
public:
	GAMEFRAMEWORK_API PongBall(GameFramework* game, DirectX::XMFLOAT3 startOffset, float radius, int ballEdgesCount, float startSpeed, float racketHitSpeedMultiplier);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;
	MulticastDelegate<bool> BallEnteredGoal;

	ID3DBlob* vertexBC;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* pixelBC;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	std::vector<DirectX::XMFLOAT4> points;
	int pointsLen;
	std::vector<int> indices;
	int indicesLen;
	DirectX::SimpleMath::Vector3 currentSpeed = { 0.0f, 0.0f, 0.0f };
	DirectX::SimpleMath::Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};
	float radius;
	float startSpeed;
	float racketHitSpeedIncrease;
	ID3D11RasterizerState* rastState;

protected:
	DirectX::SimpleMath::Vector3 RotateVectorAroundZAxis(DirectX::SimpleMath::Vector3 vector, float radians);
	void GetDeflectedFromRacket(PongRacket* racket);
};

