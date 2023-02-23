#include "PongWall.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const LPCWSTR vertexShaderPath = L"./Shaders/SimpleObjectShader.hlsl";
const LPCWSTR vertexShaderName = L"SimpleObjectShader.hlsl";

const LPCWSTR pixelShaderPath = L"./Shaders/SimpleObjectShader.hlsl";
const LPCWSTR pixelShaderName = L"SimpleObjectShader.hlsl";

PongWall::PongWall(GameFramework* game, PhysicalLayer physicalLayer, DirectX::XMFLOAT3 offset = { 0.0f, 0.0f, 0.0f }, float wallThickness = 0.025f, float wallLength = 2.0f, DirectX::XMFLOAT3 wallNormal = { 0.0f, 0.0f, 0.0f }, float yDirectionSpeedIncreaseOnBallReflect = 0.1f) : PhysicalBoxComponent(game, physicalLayer)
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
		DirectX::XMFLOAT4(wallLength  * 0.5f, wallThickness  * 0.5f, 0.0f, 1.0f),	DirectX::XMFLOAT4(0.75f, 0.75f, 0.75f, 0.75f),
		DirectX::XMFLOAT4(-wallLength * 0.5f, -wallThickness * 0.5f, 0.0f, 1.0f),	DirectX::XMFLOAT4(0.75f, 0.75f, 0.75f, 0.75f),
		DirectX::XMFLOAT4(wallLength  * 0.5f, -wallThickness * 0.5f, 0.0f, 1.0f),	DirectX::XMFLOAT4(0.75f, 0.75f, 0.75f, 0.75f),
		DirectX::XMFLOAT4(-wallLength * 0.5f, wallThickness  * 0.5f, 0.0f, 1.0f),	DirectX::XMFLOAT4(0.75f, 0.75f, 0.75f, 0.75f),
	};

	indicesLen = 6;
	indices = new int[indicesLen] { 0, 1, 2, 1, 0, 3 };

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game_->device->CreateRasterizerState(&rastDesc, &rastState);

	this->positionOffset.x = offset.x;
	this->positionOffset.y = offset.y;

	this->wallNormal = wallNormal;

	this->boundingBox.Center = offset;
	this->boundingBox.Extents = DirectX::XMFLOAT3(wallLength * 0.5f, wallThickness * 0.5f, 1.0f);
}

void PongWall::Update(float deltaTime)
{
}

void PongWall::Draw()
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
