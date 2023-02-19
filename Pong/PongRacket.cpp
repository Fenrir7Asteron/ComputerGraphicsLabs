#define NOMINMAX
#include "PongRacket.h"
#include <iostream>
#include <algorithm>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const LPCWSTR vertexShaderPath = L"./Shaders/SimpleObjectShader.hlsl";
const LPCWSTR vertexShaderName = L"SimpleObjectShader.hlsl";

const LPCWSTR pixelShaderPath = L"./Shaders/SimpleObjectShader.hlsl";
const LPCWSTR pixelShaderName = L"SimpleObjectShader.hlsl";

PongRacket::PongRacket(GameFramework* game, DirectX::XMFLOAT3 offset = { 0.0f, 0.0f, 0.0f }, float racketWidth = 0.025f, float racketLength = 0.15f, float maxSpeed = 1.0f) : PhysicalBoxComponent(game)
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
			0}
	};

	game_->device->CreateInputLayout(
		inputElements,
		2,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&layout);

	pointsLen = 8;

	points = new DirectX::XMFLOAT4[pointsLen]{
		DirectX::XMFLOAT4(racketWidth, racketLength, 0.0f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(-racketWidth, -racketLength, 0.0f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(racketWidth, -racketLength, 0.0f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(-racketWidth, racketLength, 0.0f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	};

	indicesLen = 6;
	indices = new int[indicesLen] { 0, 1, 2, 1, 0, 3 };

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game_->device->CreateRasterizerState(&rastDesc, &rastState);

	this->racketLength = racketLength;
	this->maxSpeed = maxSpeed;

	this->positionOffset.x = offset.x;
	this->positionOffset.y = offset.y;

	this->boundingBox.Center = offset;
	this->boundingBox.Extents = DirectX::XMFLOAT3(racketWidth * 0.5f, racketLength * 0.5f, 1.0f);
}

void PongRacket::Update(float deltaTime)
{
	if (game_->inputDevice->IsKeyDown(Keys::Up) && game_->inputDevice->IsKeyDown(Keys::Down)) {
		return;
	}

	if (game_->inputDevice->IsKeyDown(Keys::Up)) {
		float delta = std::min(deltaTime * maxSpeed, 1.0f - racketLength - positionOffset.y);
		Move(DirectX::SimpleMath::Vector3(0.0f, delta, 0.0f));
	} 
	else if (game_->inputDevice->IsKeyDown(Keys::Down)) {
		float delta = std::max(-deltaTime * maxSpeed, -1.0f + racketLength - positionOffset.y);
		Move(DirectX::SimpleMath::Vector3(0.0f, delta, 0.0f));
	}
}

void PongRacket::Draw()
{
	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * pointsLen;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = points;
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
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	ID3D11Buffer* ib;
	game_->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

	D3D11_BUFFER_DESC offsetBufDesc = {};
	offsetBufDesc.Usage = D3D11_USAGE_DEFAULT;
	offsetBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	offsetBufDesc.CPUAccessFlags = 0;
	offsetBufDesc.MiscFlags = 0;
	offsetBufDesc.StructureByteStride = 0;
	offsetBufDesc.ByteWidth = sizeof(DirectX::XMFLOAT4);

	D3D11_SUBRESOURCE_DATA offsetData = {};
	offsetData.pSysMem = &positionOffset;
	offsetData.SysMemPitch = 0;
	offsetData.SysMemSlicePitch = 0;

	ID3D11Buffer* constantOffsetBuffer;
	game_->device->CreateBuffer(&offsetBufDesc, &offsetData, &constantOffsetBuffer);

	UINT strides[] = { 32 };
	UINT offsets[] = { 0 };

	game_->context->RSSetState(rastState);

	game_->context->IASetInputLayout(layout);
	game_->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game_->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game_->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game_->context->VSSetConstantBuffers(0, 1, &constantOffsetBuffer);
	game_->context->VSSetShader(vertexShader, nullptr, 0);
	game_->context->PSSetShader(pixelShader, nullptr, 0);

	game_->context->OMSetRenderTargets(1, &game_->rtv, nullptr);
	game_->context->DrawIndexed(indicesLen, 0, 0);

	vb->Release();
	ib->Release();
}
