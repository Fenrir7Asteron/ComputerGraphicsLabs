#pragma once

using namespace DirectX::SimpleMath;

struct ModelViewProjectionMatrices
{
	Matrix worldMatrix;
	Matrix viewMatrix;
	Matrix projectionMatrix;
	Matrix transposeInverseWorldMatrix;
};

