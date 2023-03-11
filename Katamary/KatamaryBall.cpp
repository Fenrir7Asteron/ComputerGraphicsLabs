#define NOMINMAX
#define _USE_MATH_DEFINES
#include "KatamaryBall.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "UnlitDiffuseMaterial.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX::SimpleMath;

KatamaryBall::KatamaryBall(GameFramework* game, float radius, int verticesNPerAxis,	Vector3 position, Quaternion rotation, Vector3 scale, Material* material) 
	: GameComponent(game, nullptr, position, rotation, scale, material)
{
	float theta = 0.0f;
	float phi = 0.0f;
	float inclinationAngleStep = M_PI / ((float)(verticesNPerAxis));
	float azimuthAngleStep = 2.0f * M_PI / ((float)verticesNPerAxis);

	for (int i = 0; i < verticesNPerAxis + 1; ++i)
	{
		phi = 0.0f;

		for (int j = 0; j < verticesNPerAxis; ++j)
		{
			
			Vector4 pos = { radius * std::sin(theta) * std::cos(phi), radius * std::cos(theta), radius * std::sin(theta) * std::sin(phi), 1.0f };

			Vector4 col = { 1.0f, 1.0f, 1.0f, 1.0f };

			Vector4 norm = pos;
			norm.Normalize();

			float u = std::atan2(norm.x, norm.z) / (2 * M_PI) + 0.5;
			float v = norm.y * 0.5 + 0.5;

			points.push_back(pos);
			points.push_back(col);
			points.push_back(norm);
			points.push_back({1.0f - u, 1.0f - v, 0.0f, 0.0f});

			phi += azimuthAngleStep;
		}

		theta = std::min(theta + inclinationAngleStep, (float)M_PI);
	}

	pointsLen = (int)points.size();

	for (int i = 0; i < verticesNPerAxis; ++i)
	{
		for (int j = 0; j < verticesNPerAxis; ++j)
		{
			indices.push_back(i * verticesNPerAxis + j);
			indices.push_back(i * verticesNPerAxis + ((j + 1)));
			indices.push_back(((i + 1)) * verticesNPerAxis + j);

			indices.push_back(i * verticesNPerAxis + ((j + 1)));
			indices.push_back(((i + 1)) * verticesNPerAxis + ((j + 1)));
			indices.push_back(((i + 1)) * verticesNPerAxis + j);
		}
	}

	pointsLen = (int)points.size();

	indicesLen = (int)indices.size();

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	HRESULT res = game_->device->CreateRasterizerState(&rastDesc, &rastState);

	this->radius = radius;

	Move(Vector3::Up * radius);

	this->unlitDiffuseMaterial = dynamic_cast<UnlitDiffuseMaterial*>(material);
}

void KatamaryBall::Update(float deltaTime)
{
	
}

void KatamaryBall::Draw()
{
	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * pointsLen;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = &points[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	ID3D11Buffer* vb;
	game_->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

	D3D11_BUFFER_DESC indexBufDesc = {};
	indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufDesc.CPUAccessFlags = 0;
	indexBufDesc.MiscFlags = 0;
	indexBufDesc.StructureByteStride = 0;
	indexBufDesc.ByteWidth = sizeof(int) * indicesLen;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = &indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	ID3D11Buffer* ib;
	game_->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	D3D11_BUFFER_DESC mvpBufDesc = {};
	mvpBufDesc.Usage = D3D11_USAGE_DEFAULT;
	mvpBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mvpBufDesc.CPUAccessFlags = 0;
	mvpBufDesc.MiscFlags = 0;
	mvpBufDesc.StructureByteStride = 0;
	mvpBufDesc.ByteWidth = sizeof(ModelViewProjectionMatrices);

	ModelViewProjectionMatrices mvp;
	mvp.worldMatrix = this->GetWorldMatrix();
	mvp.viewMatrix = this->game_->camera->GetViewMatrix();
	mvp.projectionMatrix = this->game_->camera->GetProjectionMatrix();

	D3D11_SUBRESOURCE_DATA mvpData = {};
	mvpData.pSysMem = &mvp;
	mvpData.SysMemPitch = 0;
	mvpData.SysMemSlicePitch = 0;

	ID3D11Buffer* constantMvpBuffer;
	game_->device->CreateBuffer(&mvpBufDesc, &mvpData, &constantMvpBuffer);

	D3D11_BUFFER_DESC colorOffsetBufDesc = {};
	colorOffsetBufDesc.Usage = D3D11_USAGE_DEFAULT;
	colorOffsetBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	colorOffsetBufDesc.CPUAccessFlags = 0;
	colorOffsetBufDesc.MiscFlags = 0;
	colorOffsetBufDesc.StructureByteStride = 0;
	colorOffsetBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);

	UINT strides[] = { sizeof(DirectX::XMFLOAT4) * 4 };
	UINT offsets[] = { 0 };

	game_->context->RSSetState(rastState);

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

	vb->Release();
	ib->Release();
	constantMvpBuffer->Release();
}