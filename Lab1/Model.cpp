#define NOMINMAX
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include "Model.h"
#include "GameFramework.h"
#include "DebugRenderSysImpl.h"
#include "UnlitDiffuseMaterial.h"
#include <cmath>
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API Model<BoundingOrientedBox>::Model(GameFramework* game, GameComponent* parent, DirectX::SimpleMath::Matrix transform,
    const std::string modelDir, const std::string modelName,
    const LPCWSTR shaderPath, const LPCWSTR depthShaderPath,
    float startScale,
    Material* material,
    const PhongCoefficients phongCoefficients,
    PhysicalLayer physicalLayer) : GameComponent(game, parent, transform, material)
{
    this->modelDir = modelDir;
    this->shaderPath = shaderPath;
    this->depthShaderPath = depthShaderPath;
    this->phongCoefficients = phongCoefficients;

    Assimp::Importer importer;

    importer.SetPropertyBool("AI_CONFIG_PP_FD_CHECKAREA", false);

    auto pScene = importer.ReadFile(modelDir + modelName,
        aiProcessPreset_TargetRealtime_Fast
        //aiProcess_Triangulate
    );

    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        meshPtrs.push_back(ParseMesh(game, *pScene->mMeshes[i], startScale, material, pScene->mMaterials));
    }

    pRoot = ParseNode(*pScene->mRootNode);

    invalidateBoundingBox = true;
    UpdateBoundingBoxBorders(pRoot.get(), Matrix::Identity);

    BoundingBox aabbBoundingBox;
    BoundingBox::CreateFromPoints(aabbBoundingBox, minPos, maxPos);
    BoundingOrientedBox::CreateFromBoundingBox(physicalComponent.boundingShape, aabbBoundingBox);
    physicalComponent.boundingShape.Transform(physicalComponent.boundingShape, transform);
    physicalComponent.parent = this;
}

GAMEFRAMEWORK_API Model<BoundingSphere>::Model(GameFramework* game, GameComponent* parent, DirectX::SimpleMath::Matrix transform,
    const std::string modelDir, const std::string modelName,
    const LPCWSTR shaderPath, const LPCWSTR depthShaderPath,
    float startScale,
    Material* material,
    const PhongCoefficients phongCoefficients,
    PhysicalLayer physicalLayer) : GameComponent(game, parent, transform, material)
{
    this->modelDir = modelDir;
    this->shaderPath = shaderPath;
    this->depthShaderPath = depthShaderPath;
    this->phongCoefficients = phongCoefficients;

    Assimp::Importer importer;

    importer.SetPropertyBool("AI_CONFIG_PP_FD_CHECKAREA", false);

    auto pScene = importer.ReadFile(modelDir + modelName,
        aiProcessPreset_TargetRealtime_Fast
        //aiProcess_Triangulate
    );

    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        meshPtrs.push_back(ParseMesh(game, *pScene->mMeshes[i], startScale, material, pScene->mMaterials));
    }

    pRoot = ParseNode(*pScene->mRootNode);

    invalidateBoundingBox = true;
    UpdateBoundingBoxBorders(pRoot.get(), transform);

    BoundingBox aabbBoundingBox;
    BoundingBox::CreateFromPoints(aabbBoundingBox, minPos, maxPos);
    physicalComponent.boundingShape.Radius = aabbBoundingBox.Extents.x;
    physicalComponent.parent = this;
}

template <class T>
std::unique_ptr<Mesh> Model<T>::ParseMesh(GameFramework* game, const aiMesh& mesh, float startScale, Material* material, const aiMaterial* const* pMaterials)
{
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    vertices.reserve(mesh.mNumVertices);
    indices.reserve(mesh.mNumFaces * 3);

    bool useTexture = false;

    if (mesh.mMaterialIndex >= 0)
    {
        using namespace std::string_literals;
        auto& pMaterial = *pMaterials[mesh.mMaterialIndex];
        aiString texFileName;

        if (pMaterial.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
        {
            texFileName = modelDir + texFileName.C_Str();
            wchar_t wtext[255];
            std::mbstowcs(wtext, texFileName.C_Str(), texFileName.length);
            wtext[texFileName.length] = '\0';

            material = new UnlitDiffuseMaterial(shaderPath, shaderPath, this->depthShaderPath, game->device, game->displayWin, wtext);
            useTexture = true;
        }
        else if (pMaterial.GetTexture(aiTextureType_BASE_COLOR, 0, &texFileName) == aiReturn_SUCCESS)
        {
            texFileName = modelDir + texFileName.C_Str();
            wchar_t wtext[255];
            std::mbstowcs(wtext, texFileName.C_Str(), texFileName.length);
            wtext[texFileName.length] = '\0';

            material = new UnlitDiffuseMaterial(shaderPath, shaderPath, this->depthShaderPath, game->device, game->displayWin, wtext);
            useTexture = true;
        }
    }

    for (unsigned int i = 0; i < mesh.mNumVertices; ++i)
    {
        Vector4 norm = Vector4::Zero;
        Vector4 col = Vector4::One;
        Vector4 tex = Vector4::Zero;

        if (mesh.mNormals != nullptr)
        {
            norm = *reinterpret_cast<DirectX::XMFLOAT4*>(&mesh.mNormals[i]);
            norm.w = 0.0f;
        }


        if (mesh.mColors != nullptr && mesh.mColors[0] != nullptr)
            col = *reinterpret_cast<DirectX::XMFLOAT4*>(&mesh.mColors[0][i]);

        if (mesh.mTextureCoords != nullptr && mesh.mTextureCoords[0] != nullptr)
        {
            tex = *reinterpret_cast<DirectX::XMFLOAT4*>(&mesh.mTextureCoords[0][i]);
            tex.y = 1.0f - tex.y;
            tex.z = useTexture;
            tex.w = 0;
        }

        Vector4 pos = { mesh.mVertices[i].x * startScale, mesh.mVertices[i].y * startScale, mesh.mVertices[i].z * startScale, 1.0f };

        vertices.push_back({ pos, col, norm, tex });
    }

    for (unsigned int i = 0; i < mesh.mNumFaces; ++i)
    {
        const auto& faces = mesh.mFaces[i];
        assert(faces.mNumIndices == 3);

        indices.push_back(faces.mIndices[0]);
        indices.push_back(faces.mIndices[1]);
        indices.push_back(faces.mIndices[2]);
    }

    return std::make_unique<Mesh>(game, Matrix::Identity, material, vertices, indices);
}

template <class T>
std::unique_ptr<Node> Model<T>::ParseNode(const aiNode& node)
{
    Matrix transform = *reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation);
    transform.Transpose();

    std::vector<Mesh*> curMeshPtrs;
    curMeshPtrs.reserve(node.mNumMeshes);
    for (int i = 0; i < node.mNumMeshes; ++i)
    {
        const int meshIdx = node.mMeshes[i];
        curMeshPtrs.push_back(this->meshPtrs.at(meshIdx).get());
    }

    auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);

    for (int i = 0; i < node.mNumChildren; ++i)
    {
        pNode->AddChild(ParseNode(*node.mChildren[i]));
    }

    return pNode;
}

template <class T>
void Model<T>::UpdateBoundingBoxBorders(Node* pNode, Matrix accumulatedTransform)
{
    accumulatedTransform *= pNode->transform;
    accumulatedTransform.Transpose();

    for (int i = 0; i < pNode->childPtrs.size(); ++i)
    {
        UpdateBoundingBoxBorders(pNode->childPtrs[i].get(), accumulatedTransform);
    }

    for (int meshIdx = 0; meshIdx < pNode->meshPtrs.size(); ++meshIdx)
    {
        Mesh* pMesh = pNode->meshPtrs[meshIdx];

        for (int vertexIdx = 0; vertexIdx < pMesh->verticesLen; ++vertexIdx)
        {
            Vector4 pos = Vector4::Transform(pMesh->vertices[vertexIdx].pos, accumulatedTransform);

            if (invalidateBoundingBox)
            {
                minPos = pos;
                maxPos = pos;
                invalidateBoundingBox = false;
            }
            else 
            {
                minPos = XMVectorMin(minPos, pos);
                maxPos = XMVectorMax(maxPos, pos);
            }
        }
    }
}

template <class T>
void Model<T>::ElementWiseMin(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2)
{
    out.x = std::min(vec1.x, vec2.x);
    out.y = std::min(vec1.y, vec2.y);
    out.z = std::min(vec1.z, vec2.z);
    out.w = std::min(vec1.w, vec2.w);
}

template <class T>
void Model<T>::ElementWiseMax(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2)
{
    out.x = std::max(vec1.x, vec2.x);
    out.y = std::max(vec1.y, vec2.y);
    out.z = std::max(vec1.z, vec2.z);
    out.w = std::max(vec1.w, vec2.w);
}

template <class T>
void Model<T>::Move(DirectX::SimpleMath::Vector3 positionDelta)
{
    GameComponent::Move(positionDelta);
    physicalComponent.Move(positionDelta);
}

template <class T>
void Model<T>::Rotate(DirectX::SimpleMath::Vector3 axis, float angle)
{
    GameComponent::Rotate(axis, angle);
    physicalComponent.Rotate(axis, angle);
}

template <class T>
void Model<T>::RotateAroundPoint(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 axis, float angle)
{
    GameComponent::RotateAroundPoint(point, axis, angle);
    physicalComponent.RotateAroundPoint(point, axis, angle);
}

template <class T>
void Model<T>::Update(float deltaTime)
{

}

template <class T>
GAMEFRAMEWORK_API void Model<T>::DrawShadowMap()
{
    pRoot->DrawShadowMap(GetWorldMatrix());
}

template GAMEFRAMEWORK_API void Model<BoundingOrientedBox>::DrawShadowMap();
template GAMEFRAMEWORK_API void Model<BoundingSphere>::DrawShadowMap();

GAMEFRAMEWORK_API void Model<BoundingOrientedBox>::Draw()
{
    pRoot->Draw(GetWorldMatrix(), this->phongCoefficients);

    if (enabled)
        game_->debugRender->DrawOrientedBoundingBox(physicalComponent.boundingShape, Matrix::Identity);
}

GAMEFRAMEWORK_API void Model<BoundingSphere>::Draw()
{
    pRoot->Draw(GetWorldMatrix(), this->phongCoefficients);

    if (enabled)
        game_->debugRender->DrawSphere(physicalComponent.boundingShape.Radius, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(physicalComponent.boundingShape.Center), 16);
}