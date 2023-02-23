#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3d11.h>
#include "GameFramework.h"
#include "PongBall.h"

class PongRacket : public PhysicalBoxComponent
{
public:
	GAMEFRAMEWORK_API PongRacket(GameFramework* game, PhysicalLayer physicalLayer, DirectX::XMFLOAT3 offset, float racketWidth, float racketLength, float maxSpeed, float maxDeflectionDegree, bool aiControlled, PongBall* pongBall);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;
	float GetBallDeflectionDegrees(DirectX::SimpleMath::Vector3 ballPosition);

	ID3DBlob* vertexBC;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* pixelBC;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	DirectX::XMFLOAT4* points;
	int pointsLen;
	int* indices;
	int indicesLen;
	float racketLength;
	float maxSpeed;
	float maxDeflectionDegree;
	ID3D11RasterizerState* rastState;
	bool aiControlled;
	PongBall* pongBall;
};

