#define NOMINMAX
#define _USE_MATH_DEFINES
#include "Mesh.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "UnlitDiffuseMaterial.h"
#include "Model.h"
#include "Vertex.h"
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
	this->unlitDiffuseMaterial = dynamic_cast<UnlitDiffuseMaterial*>(material);

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
}

void Mesh::Update(float deltaTime)
{

}

GAMEFRAMEWORK_API void Mesh::Draw()
{
	Draw(Matrix::Identity);
}

void Mesh::Draw(Matrix accumulatedTransform)
{
	D3D11_BUFFER_DESC mvpBufDesc = {};
	mvpBufDesc.Usage = D3D11_USAGE_DEFAULT;
	mvpBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mvpBufDesc.CPUAccessFlags = 0;
	mvpBufDesc.MiscFlags = 0;
	mvpBufDesc.StructureByteStride = 0;
	mvpBufDesc.ByteWidth = sizeof(ModelViewProjectionMatrices);

	ModelViewProjectionMatrices mvp;
	mvp.worldMatrix = accumulatedTransform;
	mvp.viewMatrix = this->game_->camera->GetViewMatrix();
	mvp.projectionMatrix = this->game_->camera->GetProjectionMatrix();

	D3D11_SUBRESOURCE_DATA mvpData = {};
	mvpData.pSysMem = &mvp;
	mvpData.SysMemPitch = 0;
	mvpData.SysMemSlicePitch = 0;

	ID3D11Buffer* constantMvpBuffer;
	game_->device->CreateBuffer(&mvpBufDesc, &mvpData, &constantMvpBuffer);

	UINT strides[] = { sizeof(Vertex) };
	UINT offsets[] = { 0 };

	game_->context->RSSetState(material->rastState);

	game_->context->IASetInputLayout(material->layout);
	game_->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game_->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game_->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game_->context->VSSetConstantBuffers(0, 1, &constantMvpBuffer);
	game_->context->PSSetShaderResources(0, 1, &unlitDiffuseMaterial->textureView);
	game_->context->PSSetSamplers(0, 1, unlitDiffuseMaterial->pSampler.GetAddressOf());
	game_->context->VSSetShader(material->vertexShader, nullptr, 0);
	game_->context->PSSetShader(material->pixelShader, nullptr, 0);

	game_->context->OMSetRenderTargets(1, &game_->rtv, game_->pDSV.Get());
	game_->context->DrawIndexed(indicesLen, 0, 0);

	constantMvpBuffer->Release();
}