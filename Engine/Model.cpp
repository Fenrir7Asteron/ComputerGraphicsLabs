#define NOMINMAX
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include "Model.h"
#include "GameFramework.h"
#include "Component/GameObject.h"
#include "Render/Debug/DebugRenderSysImpl.h"
#include <cmath>
#include <iostream>

using namespace DirectX;
using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API Model<BoundingOrientedBox>::Model(GameObject* gameObject) : GameComponent(gameObject)
{
}

GAMEFRAMEWORK_API Model<BoundingSphere>::Model(GameObject* gameObject) : GameComponent(gameObject)
{
}

GAMEFRAMEWORK_API void Model<BoundingOrientedBox>::Bind(
    const std::string modelDir, const std::string modelName,
    const LPCWSTR geometryPassShaderPath, const LPCWSTR depthShaderPath,
    float startScale,
    Material* material,
    const PhongCoefficients phongCoefficients,
    PhysicalLayer physicalLayer)
{
    this->m_modelDir = modelDir;
    this->m_geometryPassShaderPath = geometryPassShaderPath;
    this->m_depthShaderPath = depthShaderPath;
    this->m_phongCoefficients = phongCoefficients;

    Assimp::Importer importer;

    importer.SetPropertyBool("AI_CONFIG_PP_FD_CHECKAREA", false);

    auto pScene = importer.ReadFile(modelDir + modelName,
        aiProcessPreset_TargetRealtime_Fast
        //aiProcess_Triangulate
    );

    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        m_meshPtrs.push_back(ParseMesh(*pScene->mMeshes[i], startScale, material, pScene->mMaterials));
    }

    m_pRoot = ParseNode(*pScene->mRootNode);

    m_invalidateBoundingBox = true;
    UpdateBoundingBoxBorders(m_pRoot.get(), Matrix::Identity);

    BoundingBox aabbBoundingBox;
    BoundingBox::CreateFromPoints(aabbBoundingBox, m_minPos, m_maxPos);
    BoundingOrientedBox::CreateFromBoundingBox(m_physicalComponent.boundingShape, aabbBoundingBox);
    m_physicalComponent.boundingShape.Transform(m_physicalComponent.boundingShape, m_gameObject->GetTransform()->GetWorldMatrix());
    m_physicalComponent.parent = this;
}

GAMEFRAMEWORK_API void Model<BoundingSphere>::Bind(
    const std::string modelDir, const std::string modelName,
    const LPCWSTR geometryPassShaderPath, const LPCWSTR depthShaderPath,
    float startScale,
    Material* material,
    const PhongCoefficients phongCoefficients,
    PhysicalLayer physicalLayer)
{
    this->m_modelDir = modelDir;
    this->m_geometryPassShaderPath = geometryPassShaderPath;
    this->m_depthShaderPath = depthShaderPath;
    this->m_phongCoefficients = phongCoefficients;

    Assimp::Importer importer;

    importer.SetPropertyBool("AI_CONFIG_PP_FD_CHECKAREA", false);

    auto pScene = importer.ReadFile(modelDir + modelName,
        aiProcessPreset_TargetRealtime_Fast
        //aiProcess_Triangulate
    );

    for (int i = 0; i < pScene->mNumMeshes; ++i)
    {
        m_meshPtrs.push_back(ParseMesh(*pScene->mMeshes[i], startScale, material, pScene->mMaterials));
    }

    m_pRoot = ParseNode(*pScene->mRootNode);

    m_invalidateBoundingBox = true;
    UpdateBoundingBoxBorders(m_pRoot.get(), m_gameObject->GetTransform()->GetWorldMatrix());

    BoundingBox aabbBoundingBox;
    BoundingBox::CreateFromPoints(aabbBoundingBox, m_minPos, m_maxPos);
    m_physicalComponent.boundingShape.Transform(m_physicalComponent.boundingShape, m_gameObject->GetTransform()->GetWorldMatrix());
    m_physicalComponent.boundingShape.Radius = aabbBoundingBox.Extents.x;
    m_physicalComponent.parent = this;
}

template <class T>
GAMEFRAMEWORK_API std::unique_ptr<Mesh> Model<T>::ParseMesh(const aiMesh& mesh, float startScale, Material* material, const aiMaterial* const* pMaterials)
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
            texFileName = m_modelDir + texFileName.C_Str();
            wchar_t wtext[255];
            std::mbstowcs(wtext, texFileName.C_Str(), texFileName.length);
            wtext[texFileName.length] = '\0';

            material = new Material(m_geometryPassShaderPath, this->m_depthShaderPath, GameFramework::Instance->device, GameFramework::Instance->displayWin, wtext);
            useTexture = true;
        }
        else if (pMaterial.GetTexture(aiTextureType_BASE_COLOR, 0, &texFileName) == aiReturn_SUCCESS)
        {
            texFileName = m_modelDir + texFileName.C_Str();
            wchar_t wtext[255];
            std::mbstowcs(wtext, texFileName.C_Str(), texFileName.length);
            wtext[texFileName.length] = '\0';

            material = new Material(m_geometryPassShaderPath, this->m_depthShaderPath, GameFramework::Instance->device, GameFramework::Instance->displayWin, wtext);
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

    return std::make_unique<Mesh>(m_gameObject, Matrix::Identity, material, vertices, indices);
}

template <class T>
GAMEFRAMEWORK_API std::unique_ptr<Node> Model<T>::ParseNode(const aiNode& node)
{
    Matrix transform = *reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation);
    transform.Transpose();

    std::vector<Mesh*> curMeshPtrs;
    curMeshPtrs.reserve(node.mNumMeshes);
    for (int i = 0; i < node.mNumMeshes; ++i)
    {
        const int meshIdx = node.mMeshes[i];
        curMeshPtrs.push_back(this->m_meshPtrs.at(meshIdx).get());
    }

    auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);

    for (int i = 0; i < node.mNumChildren; ++i)
    {
        pNode->AddChild(ParseNode(*node.mChildren[i]));
    }

    return pNode;
}

template <class T>
GAMEFRAMEWORK_API void Model<T>::UpdateBoundingBoxBorders(Node* pNode, Matrix accumulatedTransform)
{
    accumulatedTransform *= pNode->transform;
    accumulatedTransform.Transpose();

    for (int i = 0; i < pNode->childPtrs.size(); ++i)
    {
        UpdateBoundingBoxBorders(pNode->childPtrs[i].get(), accumulatedTransform);
    }

    for (int meshIdx = 0; meshIdx < pNode->m_meshPtrs.size(); ++meshIdx)
    {
        Mesh* pMesh = pNode->m_meshPtrs[meshIdx];

        for (int vertexIdx = 0; vertexIdx < pMesh->verticesLen; ++vertexIdx)
        {
            Vector4 pos = Vector4::Transform(pMesh->vertices[vertexIdx].pos, accumulatedTransform);

            if (m_invalidateBoundingBox)
            {
                m_minPos = pos;
                m_maxPos = pos;
                m_invalidateBoundingBox = false;
            }
            else 
            {
                m_minPos = XMVectorMin(m_minPos, pos);
                m_maxPos = XMVectorMax(m_maxPos, pos);
            }
        }
    }
}

template <class T>
GAMEFRAMEWORK_API void Model<T>::ElementWiseMin(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2)
{
    out.x = std::min(vec1.x, vec2.x);
    out.y = std::min(vec1.y, vec2.y);
    out.z = std::min(vec1.z, vec2.z);
    out.w = std::min(vec1.w, vec2.w);
}

template <class T>
GAMEFRAMEWORK_API void Model<T>::ElementWiseMax(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2)
{
    out.x = std::max(vec1.x, vec2.x);
    out.y = std::max(vec1.y, vec2.y);
    out.z = std::max(vec1.z, vec2.z);
    out.w = std::max(vec1.w, vec2.w);
}

template <class T>
GAMEFRAMEWORK_API void Model<T>::Update(float deltaTime)
{

}

template<class T>
GAMEFRAMEWORK_API void Model<T>::Destroy()
{
}

template <class T>
GAMEFRAMEWORK_API void Model<T>::DrawShadowMap()
{
    if (!enabled)
        return;

    m_pRoot->DrawShadowMap(m_gameObject->GetTransform()->GetWorldMatrix());
}

template GAMEFRAMEWORK_API void Model<BoundingOrientedBox>::DrawShadowMap();
template GAMEFRAMEWORK_API void Model<BoundingSphere>::DrawShadowMap();

GAMEFRAMEWORK_API void Model<BoundingOrientedBox>::GeometryPass()
{
    if (!enabled)
        return;

    m_pRoot->GeometryPass(m_gameObject->GetTransform()->GetWorldMatrix(), this->m_phongCoefficients);

    GameFramework::Instance->debugRender->DrawOrientedBoundingBox(m_physicalComponent.boundingShape, Matrix::Identity);
}

GAMEFRAMEWORK_API void Model<BoundingSphere>::GeometryPass()
{
    if (!enabled)
        return;

    m_pRoot->GeometryPass(m_gameObject->GetTransform()->GetWorldMatrix(), this->m_phongCoefficients);

    GameFramework::Instance->debugRender->DrawSphere(m_physicalComponent.boundingShape.Radius, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(m_physicalComponent.boundingShape.Center), 16);
}
