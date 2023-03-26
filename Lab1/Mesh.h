#pragma once
#include <d3d11.h>
#include "GameComponent.h"
#include <vector>
#include <string>

class UnlitDiffuseMaterial;
class Vertex;
class PhongCoefficients;

class Mesh : public GameComponent
{
public:
	GAMEFRAMEWORK_API Mesh(GameFramework* game,
		DirectX::SimpleMath::Matrix transform,
		Material* material,
		std::vector<Vertex>& vertices,
		std::vector<int>& indices);

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void DrawShadowMap() override;
	GAMEFRAMEWORK_API virtual void DrawShadowMap(DirectX::SimpleMath::Matrix accumulatedTransform);
	GAMEFRAMEWORK_API virtual void Draw() override;
	GAMEFRAMEWORK_API virtual void Draw(DirectX::SimpleMath::Matrix accumulatedTransform, const PhongCoefficients& phongCoefficients);

public:
	std::vector<Vertex> vertices;
	std::vector<int> indices;
	int verticesLen;
	int indicesLen;

	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	ID3D11Buffer* constantMvpBuffer;
	ID3D11Buffer* constantPhongBuffer;

	UnlitDiffuseMaterial* unlitDiffuseMaterial;
};

