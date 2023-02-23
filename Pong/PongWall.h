#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3d11.h>
#include "GameFramework.h"

class PongWall: public PhysicalBoxComponent
{
public:
	GAMEFRAMEWORK_API PongWall(GameFramework* game, PhysicalLayer physicalLayer, DirectX::XMFLOAT3 offset, float wallThickness, float wallLength, DirectX::XMFLOAT3 wallNormal, float yDirectionSpeedIncreaseOnBallReflect);

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
	DirectX::SimpleMath::Vector3 wallNormal;
	ID3D11RasterizerState* rastState;
	float yDirectionSpeedIncreaseOnBallReflect;
};