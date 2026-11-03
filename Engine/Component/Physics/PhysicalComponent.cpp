#include "PhysicalComponent.h"
#include "Component/GameObject.h"
#include "Node.h"
#include "Mesh.h"
#include "Model.h"
#include "Vertex.h"
#include <cmath>

using namespace DirectX;
using namespace DirectX::SimpleMath;


GAMEFRAMEWORK_API PhysicalComponent::PhysicalComponent(GameObject* gameObject) : GameComponent(gameObject)
{
	m_subscriptionId = m_gameObject->GetTransform()->SubscribeToTransformChange([this]() { InvalidateCollider(); });
}

GAMEFRAMEWORK_API void PhysicalComponent::Bind(Model* model, PhysicalLayer physicalLayer)
{
	m_invalidateAABB = true;
	UpdateAABBBoxBorders(model->GetRoot(), Matrix::Identity);

	BoundingBox::CreateFromPoints(aabbBoundingBox, m_aabbMinPos, m_aabbMaxPos);
	InvalidateCollider();
}

GAMEFRAMEWORK_API void PhysicalComponent::Update(float deltaTime)
{
	if (!m_invalidateCollider)
		return;

	UpdateCollider();
	m_invalidateCollider = false;
}

GAMEFRAMEWORK_API void PhysicalComponent::Destroy()
{
	m_gameObject->GetTransform()->UnsubscribeFromTransformChange(m_subscriptionId);
}

GAMEFRAMEWORK_API void PhysicalComponent::DrawShadowMap()
{
}

GAMEFRAMEWORK_API void PhysicalComponent::UpdateAABBBoxBorders(const Node* pNode, Matrix accumulatedTransform)
{
	accumulatedTransform *= pNode->transform;
	accumulatedTransform.Transpose();

	for (int i = 0; i < pNode->childPtrs.size(); ++i)
	{
		UpdateAABBBoxBorders(pNode->childPtrs[i].get(), accumulatedTransform);
	}

	for (int meshIdx = 0; meshIdx < pNode->m_meshPtrs.size(); ++meshIdx)
	{
		Mesh* pMesh = pNode->m_meshPtrs[meshIdx];

		for (int vertexIdx = 0; vertexIdx < pMesh->verticesLen; ++vertexIdx)
		{
			Vector4 pos = Vector4::Transform(pMesh->vertices[vertexIdx].pos, accumulatedTransform);

			if (m_invalidateAABB)
			{
				m_aabbMinPos = pos;
				m_aabbMaxPos = pos;
				m_invalidateAABB = false;
			}
			else
			{
				m_aabbMinPos = XMVectorMin(m_aabbMinPos, pos);
				m_aabbMaxPos = XMVectorMax(m_aabbMaxPos, pos);
			}
		}
	}
}


GAMEFRAMEWORK_API void PhysicalComponent::InvalidateCollider()
{
	m_invalidateCollider = true;
}
