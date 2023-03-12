#pragma once
#include <vector>
#include <memory>
#include "SimpleMath.h"

class Mesh;

class Node
{
	friend class Model;

public:
	Node(std::vector<Mesh*> meshPtrs, const DirectX::SimpleMath::Matrix& transform);
	void Draw(DirectX::SimpleMath::Matrix accumulatedTransform);

private:
	void AddChild(std::unique_ptr<Node> pChild);

private:
	std::vector<std::unique_ptr<Node>> childPtrs;
	std::vector<Mesh*> meshPtrs;
	DirectX::SimpleMath::Matrix transform;
};

