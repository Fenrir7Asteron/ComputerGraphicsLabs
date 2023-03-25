#pragma once
#include "GameFrameworkExports.h"
#include "PhysicalComponent.h"
#include "SimpleMath.h"
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
class Model : public GameComponent
{
public:
	GAMEFRAMEWORK_API Model(GameFramework* game, GameComponent* parent, DirectX::SimpleMath::Matrix transform, const std::string modelDir, const std::string modelName, const LPCWSTR shaderPath, float startScale,
		Material* material,
		const PhongCoefficients phongCoefficients,
		PhysicalLayer physicalLayer = PhysicalLayer::Default);

	std::unique_ptr<Mesh> ParseMesh(GameFramework* game, const aiMesh& mesh, float startScale, Material* material, const aiMaterial* const* pMaterials);
	std::unique_ptr<Node> ParseNode(const aiNode& node);
	void UpdateBoundingBoxBorders(Node* pNode, DirectX::SimpleMath::Matrix accumulatedTransform);
	void ElementWiseMin(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);
	void ElementWiseMax(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2);

	GAMEFRAMEWORK_API virtual void Move(DirectX::SimpleMath::Vector3 positionDelta) override;
	GAMEFRAMEWORK_API virtual void Rotate(DirectX::SimpleMath::Vector3 axis, float angle) override;
	GAMEFRAMEWORK_API virtual void RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle) override;
	GAMEFRAMEWORK_API void Update(float deltaTime) override;
	GAMEFRAMEWORK_API void Draw() override;

	PhysicalComponent<T> physicalComponent;
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

