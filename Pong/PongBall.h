#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3d11.h>
#include "PhysicalBoxComponent.h"

class SimpleMath;

class PongBall: public PhysicalBoxComponent
{
public:
	GAMEFRAMEWORK_API PongBall(GameFramework* game, DirectX::XMFLOAT3 startOffset, float radius, float startSpeed, float racketHitSpeedMultiplier);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;

	ID3DBlob* vertexBC;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* pixelBC;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	DirectX::XMFLOAT4* points;
	int pointsLen;
	int* indices;
	int indicesLen;
	DirectX::SimpleMath::Vector3 currentSpeed = { 0.0f, 0.0f, 0.0f };
	float radius;
	float startSpeed;
	float racketHitSpeedMultiplier;
	ID3D11RasterizerState* rastState;
};

