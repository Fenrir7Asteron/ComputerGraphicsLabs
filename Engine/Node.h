#pragma once
#include <vector>
#include <memory>
#include "Utils/SimpleMath.h"

class Mesh;
class PhongCoefficients;

class Node
{
public:
	Node(std::vector<Mesh*> meshPtrs, const DirectX::SimpleMath::Matrix& transform);
	void DrawShadowMap(DirectX::SimpleMath::Matrix accumulatedTransform);
	void GeometryPass(DirectX::SimpleMath::Matrix accumulatedTransform, const PhongCoefficients& phongCoefficients);

	void AddChild(std::unique_ptr<Node> pChild);

	std::vector<std::unique_ptr<Node>> childPtrs;
	std::vector<Mesh*> m_meshPtrs;
	DirectX::SimpleMath::Matrix transform;
};

