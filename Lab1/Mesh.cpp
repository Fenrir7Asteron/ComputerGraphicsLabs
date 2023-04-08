#define NOMINMAX
#define _USE_MATH_DEFINES
#include "Mesh.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "Model.h"
#include "Vertex.h"
#include "PhongCoefficients.h"
#include "PhongConstantData.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX::SimpleMath;

Mesh::Mesh(GameFramework* game, Matrix transform, Material* material,
	std::vector<Vertex>& vertices, std::vector<int>& indices)
	: GameComponent(game, nullptr, transform, material)
{
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

	game_->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

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

	game_->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

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

	mvp.viewMatrix = this->game_->camera->GetViewMatrix();
	mvp.projectionMatrix = this->game_->camera->GetProjectionMatrix();

	D3D11_SUBRESOURCE_DATA mvpData = {};
	mvpData.pSysMem = &mvp;
	mvpData.SysMemPitch = 0;
	mvpData.SysMemSlicePitch = 0;

	game_->device->CreateBuffer(&mvpBufDesc, &mvpData, &constantMvpBuffer);

	// Create Phong directional light constant buffer
	D3D11_BUFFER_DESC phongBufDesc = {};
	phongBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	phongBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	phongBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	phongBufDesc.MiscFlags = 0;
	phongBufDesc.StructureByteStride = 0;
	phongBufDesc.ByteWidth = sizeof(PhongConstantData);

	game_->device->CreateBuffer(&phongBufDesc, nullptr, &constantPhongBuffer);
}

void Mesh::Update(float deltaTime)
{

}

GAMEFRAMEWORK_API void Mesh::DrawShadowMap()
{
	DrawShadowMap(Matrix::Identity);
}

void Mesh::DrawShadowMap(Matrix accumulatedTransform)
{
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

	mvp.viewMatrix = this->game_->camera->GetViewMatrix();
	mvp.projectionMatrix = this->game_->camera->GetProjectionMatrix();

	game_->context->Map(constantMvpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &mvp, sizeof(mvp));
	game_->context->Unmap(constantMvpBuffer, 0);

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	game_->context->IASetInputLayout(material->shadowLayout);
	game_->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game_->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game_->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game_->context->VSSetConstantBuffers(1, 1, &constantMvpBuffer);
	game_->context->VSSetShader(material->shadowMappingVertexShader, nullptr, 0);
	game_->context->GSSetShader(material->shadowMappingGeometryShader, nullptr, 0);

	game_->context->DrawIndexed(indicesLen, 0, 0);
}

GAMEFRAMEWORK_API void Mesh::GeometryPass()
{
	GeometryPass(Matrix::Identity, {});
}

void Mesh::GeometryPass(Matrix accumulatedTransform, const PhongCoefficients& phongCoefficients)
{
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

	mvp.viewMatrix = this->game_->camera->GetViewMatrix();
	mvp.projectionMatrix = this->game_->camera->GetProjectionMatrix();

	game_->context->Map(constantMvpBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &mvp, sizeof(mvp));
	game_->context->Unmap(constantMvpBuffer, 0);


	// Update Phong directional light constant buffer
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	game_->context->Map(constantPhongBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &phongCoefficients, sizeof(phongCoefficients));
	game_->context->Unmap(constantPhongBuffer, 0);

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	game_->context->RSSetState(material->rastState);

	game_->context->IASetInputLayout(material->geometryLayout);
	game_->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game_->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game_->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game_->context->VSSetConstantBuffers(0, 1, &constantMvpBuffer);
	game_->context->PSSetConstantBuffers(1, 1, &constantPhongBuffer);

	game_->context->PSSetShaderResources(0, 1, &material->textureView);
	game_->context->PSSetSamplers(0, 1, material->pSampler.GetAddressOf());

	game_->context->VSSetShader(material->geometryPassVertexShader, nullptr, 0);
	game_->context->PSSetShader(material->geometryPassPixelShader, nullptr, 0);

	game_->context->DrawIndexed(indicesLen, 0, 0);
}