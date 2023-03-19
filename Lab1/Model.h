#pragma once
#include "GameFrameworkExports.h"
#include "PhysicalBoxComponent.h"
#include "SimpleMath.h"
#include <string>
#include <vector>
#include <wtypes.h>
#include <memory>
#include "directxmath.h"
#include "Node.h"
#include "PhongCoefficients.h"

class GameFramework;
class Material;
class aiMesh;
class aiNode;
class aiMaterial;
class Mesh;

class Model : public PhysicalBoxComponent
{
public:
	GAMEFRAMEWORK_API Model(GameFramework* game, GameComponent* parent, DirectX::SimpleMath::Matrix transform, const std::string modelDir, const std::string modelName, const LPCWSTR shaderPath, float startScale, Material* material, const PhongCoefficients phongCoefficients, PhysicalLayer physicalLayer = PhysicalLayer::Default);
	GAMEFRAMEWORK_API std::unique_ptr<Mesh> ParseMesh(GameFramework* game, const aiMesh& mesh, float startScale, Material* material, const aiMaterial* const* pMaterials);
	GAMEFRAMEWORK_API std::unique_ptr<Node> ParseNode(const aiNode& node);
	GAMEFRAMEWORK_API void UpdateBoundingBoxBorders(Node* pNode, DirectX::SimpleMath::Matrix accumulatedTransform);
	GAMEFRAMEWORK_API void ElementWiseMin(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);
	GAMEFRAMEWORK_API void ElementWiseMax(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);
	GAMEFRAMEWORK_API void Update(float deltaTime) override;
	GAMEFRAMEWORK_API void Draw() override;

	bool invalidateBoundingBox;
	DirectX::SimpleMath::Vector4 minPos;
	DirectX::SimpleMath::Vector4 maxPos;

private:
	std::unique_ptr<Node> pRoot;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
	std::string modelDir;
	LPCWSTR shaderPath;
	PhongCoefficients phongCoefficients;
};

