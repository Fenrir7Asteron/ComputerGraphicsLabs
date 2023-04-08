#include "Node.h"
#include "Mesh.h"
#include "PhongCoefficients.h"

Node::Node(std::vector<Mesh*> meshPtrs, const DirectX::SimpleMath::Matrix& transform) : meshPtrs(std::move(meshPtrs))
{
	this->transform = transform;
}

void Node::DrawShadowMap(DirectX::SimpleMath::Matrix accumulatedTransform)
{
	const auto currentTransform = transform * accumulatedTransform;

	for (const auto pm : meshPtrs)
	{
		pm->DrawShadowMap(currentTransform);
	}

	for (const auto& pc : childPtrs)
	{
		pc->DrawShadowMap(currentTransform);
	}
}

void Node::GeometryPass(DirectX::SimpleMath::Matrix accumulatedTransform, const PhongCoefficients& phongCoefficients)
{
	const auto currentTransform = transform * accumulatedTransform;

	for (const auto pm : meshPtrs)
	{
		pm->GeometryPass(currentTransform, phongCoefficients);
	}

	for (const auto& pc : childPtrs)
	{
		pc->GeometryPass(currentTransform, phongCoefficients);
	}
}

void Node::AddChild(std::unique_ptr<Node> pChild)
{
	childPtrs.push_back(std::move(pChild));
}
