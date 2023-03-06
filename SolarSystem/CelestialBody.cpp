#define NOMINMAX
#define _USE_MATH_DEFINES
#include "CelestialBody.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const LPCWSTR vertexShaderPath = L"./Shaders/CelestialBodyShader.hlsl";
const LPCWSTR vertexShaderName = L"CelestialBodyShader.hlsl";

const LPCWSTR pixelShaderPath = L"./Shaders/CelestialBodyShader.hlsl";
const LPCWSTR pixelShaderName = L"CelestialBodyShader.hlsl";

using namespace DirectX::SimpleMath;

CelestialBody::CelestialBody(GameFramework* game, float radius, int verticesNPerAxis, float distanceFromSun,
	Vector3 startDistanceAxis, float rotationPeriodInDays, float revolutionPeriodInDays,
	CelestialBody* revolutionOrigin,
	Vector3 rotationAxis, Vector3 revolutionAxis,
	Vector3 position, Quaternion rotation, Vector3 scale) : GameComponent(game, revolutionOrigin, position, rotation, scale)
{
	vertexBC = nullptr;
	ID3DBlob* errorVertexCode = nullptr;

	HRESULT res = D3DCompileFromFile(vertexShaderPath,
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorVertexCode);

	CheckShaderCreationSuccess(res, errorVertexCode, vertexShaderName);

	pixelBC = nullptr;
	ID3DBlob* errorPixelCode = nullptr;

	res = D3DCompileFromFile(pixelShaderPath, nullptr /*macros*/, nullptr /*include*/, "PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelBC, &errorPixelCode);
	CheckShaderCreationSuccess(res, errorPixelCode, pixelShaderName);

	res = game_->device->CreateVertexShader(
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		nullptr, &vertexShader);

	if (FAILED(res))
	{
		return;
	}

	res = game_->device->CreatePixelShader(
		pixelBC->GetBufferPointer(),
		pixelBC->GetBufferSize(),
		nullptr, &pixelShader);

	if (FAILED(res))
	{
		return;
	}

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC {
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"COLOR",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0},
		D3D11_INPUT_ELEMENT_DESC {
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0}
	};

	game_->device->CreateInputLayout(
		inputElements,
		3,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&layout);

	float theta = 0.0f;
	float phi = 0.0f;
	float inclinationAngleStep = M_PI / ((float)(verticesNPerAxis));
	float azimuthAngleStep = 2.0f * M_PI / ((float)verticesNPerAxis);

	for (int i = 0; i < verticesNPerAxis + 1; ++i)
	{
		phi = 0.0f;

		for (int j = 0; j < verticesNPerAxis; ++j)
		{
			Vector4 pos = { radius * std::sin(theta) * std::cos(phi), radius * std::sin(theta) * std::sin(phi), radius * std::cos(theta), 1.0f };

			Vector4 col = { 1.0f, 1.0f, 1.0f, 1.0f };

			Vector4 norm = pos;
			norm.Normalize();

			points.push_back(pos);
			points.push_back(col);
			points.push_back(norm);
			phi += azimuthAngleStep;
		}
		
		theta = std::min(theta + inclinationAngleStep, (float) M_PI);
	}

	pointsLen = (int)points.size();

	for (int i = 0; i < verticesNPerAxis; ++i)
	{
		for (int j = 0; j < verticesNPerAxis; ++j)
		{
			indices.push_back(((i + 1)) * verticesNPerAxis + j);
			indices.push_back(i * verticesNPerAxis + ((j + 1)));
			indices.push_back(i * verticesNPerAxis + j);

			indices.push_back(((i + 1)) * verticesNPerAxis + j);
			indices.push_back(((i + 1)) * verticesNPerAxis + ((j + 1)));
			indices.push_back(i * verticesNPerAxis + ((j + 1)));
		}
	}

	pointsLen = (int)points.size();

	indicesLen = (int)indices.size();

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game_->device->CreateRasterizerState(&rastDesc, &rastState);

	Rotate(Vector3::Right, DirectX::XMConvertToRadians(90.0f));
	if (revolutionOrigin != nullptr)
		Move(startDistanceAxis * distanceFromSun);

	this->distanceFromSun = distanceFromSun;
	this->rotationSpeed = (2 * M_PI) / rotationPeriodInDays;
	this->revolutionSpeed = (2 * M_PI) / revolutionPeriodInDays;
	this->radius = radius;
	this->revolutionOrigin = revolutionOrigin;
	this->rotationAxis = rotationAxis;
	this->revolutionAxis = revolutionAxis;
}

void CelestialBody::Update(float deltaTime)
{
	Rotate(rotationAxis, deltaTime * rotationSpeed);

	if (revolutionOrigin != nullptr)
	{
		RotateAroundPoint(Vector3::Zero, revolutionAxis, deltaTime * revolutionSpeed);
	}
}

void CelestialBody::Draw()
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

	UINT strides[] = { sizeof(DirectX::XMFLOAT4) * 3 };
	UINT offsets[] = { 0 };

	game_->context->RSSetState(rastState);

	game_->context->IASetInputLayout(layout);
	game_->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game_->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game_->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game_->context->VSSetConstantBuffers(0, 1, &constantMvpBuffer);
	game_->context->VSSetShader(vertexShader, nullptr, 0);
	game_->context->PSSetShader(pixelShader, nullptr, 0);

	game_->context->OMSetRenderTargets(1, &game_->rtv, game_->pDSV.Get());
	game_->context->DrawIndexed(indicesLen, 0, 0);

	vb->Release();
	ib->Release();
	constantMvpBuffer->Release();
}