#include "StaticBox.h"
#include <ModelViewProjectionMatrices.h>
#include <UnlitDiffuseMaterial.h>
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const LPCWSTR vertexShaderPath = L"./Shaders/KatamaryShader.hlsl";
const LPCWSTR vertexShaderName = L"KatamaryShader.hlsl";

const LPCWSTR pixelShaderPath = L"./Shaders/KatamaryShader.hlsl";
const LPCWSTR pixelShaderName = L"KatamaryShader.hlsl";

using namespace DirectX::SimpleMath;

StaticBox::StaticBox(GameFramework* game, Vector3 position,	Quaternion rotation, Vector3 scale, Material* material) : GameComponent(game, nullptr, position, rotation, scale, material)
{
	float sideHalf= 50.0f;
	points =
	{
		// front
		DirectX::XMFLOAT4(- sideHalf, - sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, - sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, + sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, + sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
									   
		// right					   
		DirectX::XMFLOAT4(+ sideHalf, - sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, - sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, + sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, + sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
									   
		// back						   
		DirectX::XMFLOAT4(+ sideHalf, - sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, - sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, + sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, + sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
									   
		// left						   
		DirectX::XMFLOAT4(- sideHalf, - sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, - sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, + sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, + sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
									   
		// bottom					   
		DirectX::XMFLOAT4(- sideHalf, - sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, - sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, - sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, - sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
									   
		// top						   
		DirectX::XMFLOAT4(- sideHalf, + sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, + sideHalf, - sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(+ sideHalf, + sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(- sideHalf, + sideHalf, + sideHalf, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),	  DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
	};

	pointsLen = (int)points.size();

	indices = {
		2, 1, 0, 0, 3, 2,		// front  
		6, 5, 4, 4, 7, 6,		// right
		10, 9, 8, 8, 11, 10,	// back
		14, 13, 12, 12, 15, 14, // left
		16, 17, 18, 18, 19, 16, // bottom
		22, 21, 20, 20, 23, 22, // top
	};

	indicesLen = (int)indices.size();

	this->unlitDiffuseMaterial = dynamic_cast<UnlitDiffuseMaterial*>(material);

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

	game_->device->CreateBuffer(&indexBufDesc, &indexData, &ib);
}

void StaticBox::Update(float deltaTime)
{

}

void StaticBox::Draw()
{
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

	UINT strides[] = { sizeof(DirectX::XMFLOAT4) * 4 };
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
