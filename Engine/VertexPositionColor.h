#pragma once
#include "GameFrameworkExports.h"
#include "SimpleMath.h"
#include "d3d11.h"

struct GAMEFRAMEWORK_API VertexPositionColor
{
	VertexPositionColor() = default;
	VertexPositionColor(DirectX::SimpleMath::Vector4 position, DirectX::SimpleMath::Vector4 color) : Position(position), Color(color)
	{}
	
	DirectX::SimpleMath::Vector4 Position;
	DirectX::SimpleMath::Vector4 Color;

	static const UINT Stride = 32;

	static ID3D11InputLayout* GetLayout(ID3DBlob* signature);
};
