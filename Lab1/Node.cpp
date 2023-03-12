#include "Node.h"
#include "Mesh.h"

Node::Node(std::vector<Mesh*> meshPtrs, const DirectX::SimpleMath::Matrix& transform) : meshPtrs(std::move(meshPtrs))
{
	this->transform = transform;
}

void Node::Draw(DirectX::SimpleMath::Matrix accumulatedTransform)
{
	const auto currentTransform = transform * accumulatedTransform;

	for (const auto pm : meshPtrs)
	{
		pm->Draw(currentTransform);
	}

	for (const auto& pc : childPtrs)
	{
		pc->Draw(currentTransform);
	}
}

void Node::AddChild(std::unique_ptr<Node> pChild)
{
	childPtrs.push_back(std::move(pChild));
}
