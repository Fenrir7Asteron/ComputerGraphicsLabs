#define NOMINMAX
#define _USE_MATH_DEFINES
#include "Mesh.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "Model.h"
#include "Vertex.h"
#include "PhongCoefficients.h"
#include "PhongConstantData.h"
#include "Camera/Camera.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX::SimpleMath;

GAMEFRAMEWORK_API Mesh::Mesh(GameObject* gameObject, Matrix transform, Material* material,
	std::vector<Vertex>& vertices, std::vector<int>& indices)
	: GameComponent(gameObject)
{
	this->material = material;

	this->vertices = std::move(vertices);
	this->indices = std::move(indices);
	this->verticesLen = this->vertices.size();
	this->indicesLen = this->indices.size();

	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(Vertex) * verticesLen;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = &this->vertices[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	GameFramework::Instance->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * indicesLen;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = &this->indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	GameFramework::Instance->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	// Create MVP transform constant buffer
	D3D11_BUFFER_DESC mvpBufDesc = {};
	mvpBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	mvpBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mvpBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mvpBufDesc.MiscFlags = 0;
	mvpBufDesc.StructureByteStride = 0;
	mvpBufDesc.ByteWidth = sizeof(ModelViewProjectionMatrices);

	ModelViewProjectionMatrices mvp;
	mvp.worldMatrix = transform;

	mvp.transposeInverseWorldMatrix = {
		transform._11, transform._12, transform._13, 0.0f,
		transform._21, transform._22, transform._23, 0.0f,
		transform._31, transform._32, transform._33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	mvp.transposeInverseWorldMatrix = mvp.transposeInverseWorldMatrix.Invert().Transpose();

	mvp.viewMatrix = GameFramework::Instance->camera->GetViewMatrix();
	mvp.projectionMatrix = GameFramework::Instance->camera->GetProjectionMatrix();

	D3D11_SUBRESOURCE_DATA mvpData = {};
	mvpData.pSysMem = &mvp;
	mvpData.SysMemPitch = 0;
	mvpData.SysMemSlicePitch = 0;

	GameFramework::Instance->device->CreateBuffer(&mvpBufDesc, &mvpData, &constantMvpBuffer);

	// Create Phong directional light constant buffer
	D3D11_BUFFER_DESC phongBufDesc = {};
	phongBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	phongBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	phongBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	phongBufDesc.MiscFlags = 0;
	phongBufDesc.StructureByteStride = 0;
	phongBufDesc.ByteWidth = sizeof(PhongConstantData);

	GameFramework::Instance->device->CreateBuffer(&phongBufDesc, nullptr, &constantPhongBuffer);
}

GAMEFRAMEWORK_API void Mesh::Update(float deltaTime)
{

}

GAMEFRAMEWORK_API void Mesh::Destroy()
{
	vertices.clear();
	indices.clear();

	vb->Release();
	ib->Release();
	constantMvpBuffer->Release();
	constantPhongBuffer->Release();
}

GAMEFRAMEWORK_API void Mesh::DrawShadowMap()
{
	DrawShadowMap(Matrix::Identity);
}

void Mesh::DrawShadowMap(Matrix accumulatedTransform)
{
	auto* camera = GameFramework::Instance->camera;
	auto* context = GameFramework::Instance->context;

	// Update MVP transform constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	ModelViewProjectionMatrices mvp;
	mvp.worldMatrix = accumulatedTransform;

	mvp.transposeInverseWorldMatrix = {
		accumulatedTransform._11, accumulatedTransform._12, accumulatedTransform._13, 0.0f,
		accumulatedTransform._21, accumulatedTransform._22, accumulatedTransform._23, 0.0f,
		accumulatedTransform._31, accumulatedTransform._32, accumulatedTransform._33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	mvp.transposeInverseWorldMatrix = mvp.transposeInverseWorldMatrix.Invert().Transpose();

	mvp.viewMatrix = camera->GetViewMatrix();
	mvp.projectionMatrix = camera->GetProjectionMatrix();

	context->Map(constantMvpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &mvp, sizeof(mvp));
	context->Unmap(constantMvpBuffer, 0);

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	context->IASetInputLayout(material->shadowLayout);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	context->VSSetConstantBuffers(1, 1, &constantMvpBuffer);
	context->VSSetShader(material->shadowMappingVertexShader, nullptr, 0);
	context->GSSetShader(material->shadowMappingGeometryShader, nullptr, 0);

	context->DrawIndexed(indicesLen, 0, 0);
}

GAMEFRAMEWORK_API void Mesh::GeometryPass()
{
	GeometryPass(Matrix::Identity, {});
}

GAMEFRAMEWORK_API void Mesh::GeometryPass(Matrix accumulatedTransform, const PhongCoefficients& phongCoefficients)
{
	auto* camera = GameFramework::Instance->camera;
	auto* context = GameFramework::Instance->context;

	// Update MVP transform constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	ModelViewProjectionMatrices mvp;
	mvp.worldMatrix = accumulatedTransform;

	mvp.transposeInverseWorldMatrix = {
		accumulatedTransform._11, accumulatedTransform._12, accumulatedTransform._13, 0.0f,
		accumulatedTransform._21, accumulatedTransform._22, accumulatedTransform._23, 0.0f,
		accumulatedTransform._31, accumulatedTransform._32, accumulatedTransform._33, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	mvp.transposeInverseWorldMatrix = mvp.transposeInverseWorldMatrix.Invert().Transpose();

	mvp.viewMatrix = camera->GetViewMatrix();
	mvp.projectionMatrix = camera->GetProjectionMatrix();

	context->Map(constantMvpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &mvp, sizeof(mvp));
	context->Unmap(constantMvpBuffer, 0);


	// Update Phong directional light constant buffer
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	context->Map(constantPhongBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &phongCoefficients, sizeof(phongCoefficients));
	context->Unmap(constantPhongBuffer, 0);

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	context->RSSetState(material->rastState);

	context->IASetInputLayout(material->geometryLayout);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	context->VSSetConstantBuffers(0, 1, &constantMvpBuffer);
	context->PSSetConstantBuffers(1, 1, &constantPhongBuffer);

	context->PSSetShaderResources(0, 1, &material->textureView);
	context->PSSetSamplers(0, 1, material->pSampler.GetAddressOf());

	context->VSSetShader(material->geometryPassVertexShader, nullptr, 0);
	context->PSSetShader(material->geometryPassPixelShader, nullptr, 0);

	context->DrawIndexed(indicesLen, 0, 0);
}