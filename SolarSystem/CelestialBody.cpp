#define NOMINMAX
#define _USE_MATH_DEFINES
#include "CelestialBody.h"
#include "GameFramework.h"
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

CelestialBody::CelestialBody(GameFramework* game, Vector3 position, Quaternion rotation, Vector3 scale, float radius = 100.0f) : GameComponent(game, position, rotation, scale)
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

	// format = POSITION, COLOR, NORMAL
	points =
	{
		// front
		DirectX::XMFLOAT4(position.x - radius, position.y - radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y - radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y + radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y + radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),

		// right
		DirectX::XMFLOAT4(position.x + radius, position.y - radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y - radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y + radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y + radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),

		// back
		DirectX::XMFLOAT4(position.x + radius, position.y - radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y - radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y + radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y + radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),

		// left
		DirectX::XMFLOAT4(position.x - radius, position.y - radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y - radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y + radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y + radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f),

		// bottom
		DirectX::XMFLOAT4(position.x - radius, position.y - radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y - radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y - radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y - radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),

		// top
		DirectX::XMFLOAT4(position.x - radius, position.y + radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y + radius, position.z - radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + radius, position.y + radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - radius, position.y + radius, position.z + radius, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f),
	};

	pointsLen = (int)points.size();

	indices = { 
		0, 1, 2, 2, 3, 0,		// front  
		4, 5, 6, 6, 7, 4,		// right
		8, 9, 10, 10, 11, 8,	// back
		12, 13, 14, 14, 15, 12, // left
		16, 17, 18, 18, 19, 16, // bottom
		20, 21, 22, 22, 23, 20, // top
	};

	indicesLen = (int)indices.size();

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game_->device->CreateRasterizerState(&rastDesc, &rastState);
}

void CelestialBody::Update(float deltaTime)
{
	
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
	game_->context->VSSetConstantBuffers(0, 1, &constantOffsetBuffer);
	game_->context->VSSetShader(vertexShader, nullptr, 0);
	game_->context->PSSetShader(pixelShader, nullptr, 0);

	game_->context->OMSetRenderTargets(1, &game_->rtv, nullptr);
	game_->context->DrawIndexed(indicesLen, 0, 0);

	vb->Release();
	ib->Release();
}