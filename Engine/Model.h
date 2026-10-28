#pragma once
#include "GameFrameworkExports.h"
#include "Component/Physics/PhysicalComponent.h"
#include "Utils/SimpleMath.h"
#include <string>
#include <vector>
#include <wtypes.h>
#include <memory>
#include "directxmath.h"
#include "Node.h"
#include "Mesh.h"
#include "Vertex.h"
#include "PhongCoefficients.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class GameFramework;
class Material;

using namespace DirectX;
using namespace DirectX::SimpleMath;

template <class T>
class Model : public GameComponent, public RenderComponent
{
public:
	GAMEFRAMEWORK_API Model(GameObject* gameObject);

	GAMEFRAMEWORK_API void Bind(const std::string modelDir, const std::string modelName,
		const LPCWSTR geometryPassShaderPath, const LPCWSTR depthShaderPath,
		float startScale,
		Material* material,
		const PhongCoefficients phongCoefficients,
		PhysicalLayer physicalLayer = PhysicalLayer::Default);

	GAMEFRAMEWORK_API std::unique_ptr<Mesh> ParseMesh(const aiMesh& mesh, float startScale, Material* material, const aiMaterial* const* pMaterials);
	GAMEFRAMEWORK_API std::unique_ptr<Node> ParseNode(const aiNode& node);
	GAMEFRAMEWORK_API void UpdateBoundingBoxBorders(Node* pNode, DirectX::SimpleMath::Matrix accumulatedTransform);
	GAMEFRAMEWORK_API void ElementWiseMin(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);
	GAMEFRAMEWORK_API void ElementWiseMax(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);

	GAMEFRAMEWORK_API void UpdateBoundary();

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Destroy() override;

	GAMEFRAMEWORK_API virtual void DrawShadowMap() override;
	GAMEFRAMEWORK_API virtual void GeometryPass() override;

	PhysicalComponent<T> m_physicalComponent;
	bool m_invalidateBoundingBox;
	DirectX::SimpleMath::Vector4 m_minPos;
	DirectX::SimpleMath::Vector4 m_maxPos;
	std::vector<std::unique_ptr<Mesh>> m_meshPtrs;
	bool m_drawBoundary = true;

private:
	GAMEFRAMEWORK_API void OnTransformChange();

	std::unique_ptr<Node> m_pRoot;
	std::string m_modelDir;
	LPCWSTR m_geometryPassShaderPath;
	LPCWSTR m_depthShaderPath;
	PhongCoefficients m_phongCoefficients;
	DirectX::SimpleMath::Vector3 m_offset;
	int m_subscriptionId = -1;
};

