#pragma once
#include <d3dcompiler.h>
#include <directxmath.h>
#include <d3d11.h>
#include "GameFramework.h"

class CelestialBody : public GameComponent
{
public:
	GAMEFRAMEWORK_API CelestialBody(GameFramework* game, Vector3 position, Quaternion rotation, Vector3 scale, float radius);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Draw() override;

	ID3DBlob* vertexBC;
	ID3D11VertexShader* vertexShader;
	ID3DBlob* pixelBC;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* layout;
	std::vector<DirectX::XMFLOAT4> points;
	int pointsLen;
	std::vector<int> indices;
	int indicesLen;
	ID3D11RasterizerState* rastState;
};

