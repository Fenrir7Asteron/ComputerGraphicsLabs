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

GAMEFRAMEWORK_API Model::Model(GameObject* gameObject) : GameComponent(gameObject)
{
}


GAMEFRAMEWORK_API void Model::Bind(
    const std::string modelDir, const std::string modelName,
    const LPCWSTR geometryPassShaderPath, const LPCWSTR depthShaderPath,
    float startScale,
    Material* material,
    const PhongCoefficients phongCoefficients)
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
}

GAMEFRAMEWORK_API std::unique_ptr<Mesh> Model::ParseMesh(const aiMesh& mesh, float startScale, Material* material, const aiMaterial* const* pMaterials)
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

GAMEFRAMEWORK_API std::unique_ptr<Node> Model::ParseNode(const aiNode& node)
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

GAMEFRAMEWORK_API void Model::ElementWiseMin(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2)
{
    out.x = std::min(vec1.x, vec2.x);
    out.y = std::min(vec1.y, vec2.y);
    out.z = std::min(vec1.z, vec2.z);
    out.w = std::min(vec1.w, vec2.w);
}

GAMEFRAMEWORK_API void Model::ElementWiseMax(DirectX::SimpleMath::Vector4& out, DirectX::SimpleMath::Vector4& vec1, DirectX::SimpleMath::Vector4& vec2)
{
    out.x = std::max(vec1.x, vec2.x);
    out.y = std::max(vec1.y, vec2.y);
    out.z = std::max(vec1.z, vec2.z);
    out.w = std::max(vec1.w, vec2.w);
}

GAMEFRAMEWORK_API const Node* Model::GetRoot()
{
    return m_pRoot.get();
}

GAMEFRAMEWORK_API void Model::Update(float deltaTime)
{
}

GAMEFRAMEWORK_API void Model::Destroy()
{
}

GAMEFRAMEWORK_API void Model::DrawShadowMap()
{
    if (!enabled)
        return;

    m_pRoot->DrawShadowMap(m_gameObject->GetTransform()->GetWorldMatrix());
}

GAMEFRAMEWORK_API void Model::GeometryPass()
{
    if (!enabled)
        return;

    m_pRoot->GeometryPass(m_gameObject->GetTransform()->GetWorldMatrix(), this->m_phongCoefficients);
}
