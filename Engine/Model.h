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

class Model : public GameComponent, public RenderComponent
{
public:
	GAMEFRAMEWORK_API Model(GameObject* gameObject);

	GAMEFRAMEWORK_API void Bind(const std::string modelDir, const std::string modelName,
		const LPCWSTR geometryPassShaderPath, const LPCWSTR depthShaderPath,
		float startScale,
		Material* material,
		const PhongCoefficients phongCoefficients);

	GAMEFRAMEWORK_API const Node* GetRoot();

	// Inherited via GameComponent
	GAMEFRAMEWORK_API virtual void Update(float deltaTime) override;
	GAMEFRAMEWORK_API virtual void Destroy() override;

	GAMEFRAMEWORK_API virtual void DrawShadowMap() override;
	GAMEFRAMEWORK_API virtual void GeometryPass() override;

	std::vector<std::unique_ptr<Mesh>> m_meshPtrs;

private:
	GAMEFRAMEWORK_API std::unique_ptr<Mesh> ParseMesh(const aiMesh& mesh, float startScale, Material* material, const aiMaterial* const* pMaterials);
	GAMEFRAMEWORK_API std::unique_ptr<Node> ParseNode(const aiNode& node);
	GAMEFRAMEWORK_API void ElementWiseMin(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);
	GAMEFRAMEWORK_API void ElementWiseMax(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);

	std::unique_ptr<Node> m_pRoot;
	std::string m_modelDir;
	LPCWSTR m_geometryPassShaderPath;
	LPCWSTR m_depthShaderPath;
	PhongCoefficients m_phongCoefficients;
	DirectX::SimpleMath::Vector3 m_offset;
};

