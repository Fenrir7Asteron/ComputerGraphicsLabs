#define NOMINMAX
#define _USE_MATH_DEFINES
#include "PongBall.h"
#include "PongRacket.h"
#include "PongWall.h"
#include "GameFramework.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const LPCWSTR vertexShaderPath = L"./Shaders/BallObjectShader.hlsl";
const LPCWSTR vertexShaderName = L"BallObjectShader.hlsl";

const LPCWSTR pixelShaderPath = L"./Shaders/BallObjectShader.hlsl";
const LPCWSTR pixelShaderName = L"BallObjectShader.hlsl";

PongBall::PongBall(GameFramework* game, DirectX::XMFLOAT3 startOffset = { 0.0f, 0.0f, 0.0f }, float radius = 0.1f, int ballEdgesCount = 16, float startSpeed = 0.1f, float racketHitSpeedIncrease = 0.1f) : PhysicalBoxComponent(game)
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
			0}
	};

	game_->device->CreateInputLayout(
		inputElements,
		1,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&layout);

	float phi = 0.0f;
	float angleStep = 2.0f * M_PI / ((float)ballEdgesCount);

	for (int idx = 0; idx < ballEdgesCount; ++idx)
	{
		points.push_back({ startOffset.x + radius * std::cos(phi), startOffset.y + radius * std::sin(phi), 0.0f, 1.0f });
		phi += angleStep;
	}
	points.push_back({ startOffset.x, startOffset.y, 0.0f, 1.0f });

	pointsLen = (int) points.size();

	for (int idx = 0; idx < pointsLen - 1; ++idx)
	{
		indices.push_back(0);
		indices.push_back(idx);
		indices.push_back((idx + 1) % (pointsLen - 1));
	}

	indicesLen = (int)indices.size();

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	res = game_->device->CreateRasterizerState(&rastDesc, &rastState);

	this->radius = radius;
	this->startSpeed = startSpeed;
	this->racketHitSpeedIncrease = racketHitSpeedIncrease;
	this->currentSpeed = DirectX::SimpleMath::Vector3{ startSpeed, 0.0f, 0.0f };

	this->boundingBox.Center = startOffset;
	this->boundingBox.Extents = DirectX::XMFLOAT3(radius, radius, radius);
}

void PongBall::Update(float deltaTime)
{
	PhysicalBoxComponent* intersectedBox = game_->RayIntersectsSomething(this, positionOffset, currentSpeed);
	
	if (intersectedBox == nullptr) 
	{
		intersectedBox = game_->RayIntersectsSomething(this, DirectX::SimpleMath::Vector3{ positionOffset.x, positionOffset.y + radius, positionOffset.z}, currentSpeed);
	}

	if (intersectedBox == nullptr)
	{
		intersectedBox = game_->RayIntersectsSomething(this, DirectX::SimpleMath::Vector3{ positionOffset.x, positionOffset.y - radius, positionOffset.z }, currentSpeed);
	}

	if (intersectedBox == nullptr)
	{
		intersectedBox = game_->RayIntersectsSomething(this, DirectX::SimpleMath::Vector3{ positionOffset.x + radius, positionOffset.y, positionOffset.z }, currentSpeed);
	}

	if (intersectedBox == nullptr)
	{
		intersectedBox = game_->RayIntersectsSomething(this, DirectX::SimpleMath::Vector3{ positionOffset.x - radius, positionOffset.y, positionOffset.z }, currentSpeed);
	}

	/*if (intersectedBox == nullptr)
		intersectedBox = game_->Intersects(this);*/

	if (intersectedBox != nullptr) {
		if (intersectedBox->physicalLayer == PhysicalLayer::Player && currentSpeed.x < -0.001f) {
			PongRacket* intersectedRacket = dynamic_cast<PongRacket*>(intersectedBox);
			if (intersectedRacket != nullptr) {
				std::cout << "Ball deflected by player racket" << std::endl;
				GetDeflectedFromRacket(intersectedRacket);
			}
			
		}
		else if (intersectedBox->physicalLayer == PhysicalLayer::Enemy && currentSpeed.x > 0.001f) {
			PongRacket* intersectedRacket = dynamic_cast<PongRacket*>(intersectedBox);
			if (intersectedRacket != nullptr) {
				std::cout << "Ball deflected by enemy racket" << std::endl;
				GetDeflectedFromRacket(intersectedRacket);
			}
		}
		else {
			PongWall* intersectedWall= dynamic_cast<PongWall*>(intersectedBox);
			if (intersectedWall != nullptr) {
				std::cout << "Ball reflected by a wall" << std::endl;
				currentSpeed = DirectX::SimpleMath::Vector3::Reflect(currentSpeed, intersectedWall->wallNormal);
				currentSpeed.y += intersectedWall->wallNormal.y * intersectedWall->yDirectionSpeedIncreaseOnBallReflect;
			}
		}
	}

	Move(currentSpeed * deltaTime);	

	if (std::abs(positionOffset.x) >= 0.95f) 
	{
		BallEnteredGoal.Broadcast(positionOffset.x > 0.0f);
	}
}

void PongBall::Draw()
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

	D3D11_SUBRESOURCE_DATA colorOffsetData = {};
	colorOffsetData.pSysMem = &color;
	colorOffsetData.SysMemPitch = 0;
	colorOffsetData.SysMemSlicePitch = 0;

	ID3D11Buffer* colorConstantOffsetBuffer;
	game_->device->CreateBuffer(&colorOffsetBufDesc, &colorOffsetData, &colorConstantOffsetBuffer);

	UINT strides[] = { sizeof(DirectX::XMFLOAT4) };
	UINT offsets[] = { 0 };

	game_->context->RSSetState(rastState);

	game_->context->IASetInputLayout(layout);
	game_->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game_->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game_->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game_->context->VSSetConstantBuffers(0, 1, &constantOffsetBuffer);
	game_->context->VSSetConstantBuffers(1, 1, &colorConstantOffsetBuffer);
	game_->context->VSSetShader(vertexShader, nullptr, 0);
	game_->context->PSSetShader(pixelShader, nullptr, 0);

	game_->context->OMSetRenderTargets(1, &game_->rtv, nullptr);
	game_->context->DrawIndexed(indicesLen, 0, 0);

	vb->Release();
	ib->Release();
}

DirectX::SimpleMath::Vector3 PongBall::RotateVectorAroundZAxis(DirectX::SimpleMath::Vector3 vector, float radians)
{
	float cos = std::cos(radians);
	float sin = std::sin(radians);
	DirectX::SimpleMath::Matrix rotationMatrix(
		DirectX::SimpleMath::Vector4(cos, -sin, 0, 0),
		DirectX::SimpleMath::Vector4(sin,  cos, 0, 0),
		DirectX::SimpleMath::Vector4(  0,    0, 0, 0),
		DirectX::SimpleMath::Vector4(  0,    0, 0, 0));

	return DirectX::XMVector3Transform(vector, rotationMatrix);
}

float signnum_c(float x) {
	if (x >= 0.0) 
		return 1.0;
	else 
		return - 1.0;
}

void PongBall::GetDeflectedFromRacket(PongRacket* racket)
{
	float deflectionRadians = racket->GetBallDeflectionDegrees(positionOffset) * M_PI / 180.0f;
	float currentSpeedMagnitude = currentSpeed.Length();
	float sign = signnum_c(-currentSpeed.x);
	currentSpeed = {currentSpeedMagnitude + racketHitSpeedIncrease, 0.0f, 0.0f};
	currentSpeed = RotateVectorAroundZAxis(currentSpeed, -deflectionRadians);
	currentSpeed.x *= sign;
}
