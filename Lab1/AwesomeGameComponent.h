#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3d11.h>
#include "GameComponent.h"

class AwesomeGameComponent : public GameComponent
{
public:
	AwesomeGameComponent(Game* game, DirectX::XMFLOAT3 offset);

	// Inherited via GameComponent
	virtual void Update(float deltaTime) override;
	virtual void Draw() override;

	ID3DBlob* vertexBC;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* pixelBC;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	DirectX::XMFLOAT4* points;
	int pointsLen;
	int* indices;
	int indicesLen;
	ID3D11RasterizerState* rastState;
};
