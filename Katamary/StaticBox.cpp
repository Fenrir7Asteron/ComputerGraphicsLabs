#include "StaticBox.h"
#include <ModelViewProjectionMatrices.h>
#include <UnlitDiffuseMaterial.h>
#include <iostream>
#include <PhongConstantData.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

const LPCWSTR vertexShaderPath = L"./Shaders/KatamaryShader.hlsl";
const LPCWSTR vertexShaderName = L"KatamaryShader.hlsl";

const LPCWSTR pixelShaderPath = L"./Shaders/KatamaryShader.hlsl";
const LPCWSTR pixelShaderName = L"KatamaryShader.hlsl";

using namespace DirectX::SimpleMath;

StaticBox::StaticBox(GameFramework* game, const PhongCoefficients phongCoefficients, Matrix transform, Material* material) : GameComponent(game, nullptr, transform, material)
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

	this->phongCoefficients = phongCoefficients;

	// Create MVP transform constant buffer
	D3D11_BUFFER_DESC mvpBufDesc = {};
	mvpBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	mvpBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mvpBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mvpBufDesc.MiscFlags = 0;
	mvpBufDesc.StructureByteStride = 0;
	mvpBufDesc.ByteWidth = sizeof(ModelViewProjectionMatrices);

	ModelViewProjectionMatrices mvp;
	mvp.worldMatrix = this->GetWorldMatrix();

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

	PhongConstantData phong;
	game_->dirLight.direction.Normalize();

	phong.cameraPosition = Vector4(game_->camera->position.x, game_->camera->position.y, game_->camera->position.z, 1.0f);
	phong.direction = game_->dirLight.direction;
	phong.lightColor = game_->dirLight.lightColor;

	phong.dirLightDiffuseCoefficient = phongCoefficients.dirLightDiffuseCoefficient;
	phong.dirLightSpecularCoefficient_alpha = phongCoefficients.dirLightSpecularCoefficient_alpha;
	phong.dirLightAmbientCoefficient = phongCoefficients.dirLightAmbientCoefficient;

	phong.DSAIntensities = { game_->dirLight.diffuseIntensity, game_->dirLight.specularIntensity, game_->dirLight.ambientIntensity, 0.0f };

	D3D11_SUBRESOURCE_DATA phongData = {};
	phongData.pSysMem = &phong;
	phongData.SysMemPitch = 0;
	phongData.SysMemSlicePitch = 0;

	game_->device->CreateBuffer(&phongBufDesc, &phongData, &constantPhongBuffer);
}

void StaticBox::Update(float deltaTime)
{

}

void StaticBox::Draw()
{
	// Update MVP transform constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	ModelViewProjectionMatrices mvp;
	mvp.worldMatrix = this->GetWorldMatrix();

	mvp.transposeInverseWorldMatrix = {
		mvp.worldMatrix._11, mvp.worldMatrix._12, mvp.worldMatrix._13, 0.0f,
		mvp.worldMatrix._21, mvp.worldMatrix._22, mvp.worldMatrix._23, 0.0f,
		mvp.worldMatrix._31, mvp.worldMatrix._32, mvp.worldMatrix._33, 0.0f,
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

	PhongConstantData phong;
	game_->dirLight.direction.Normalize();

	phong.cameraPosition = Vector4(game_->camera->position.x, game_->camera->position.y, game_->camera->position.z, 1.0f);
	phong.direction = game_->dirLight.direction;
	phong.lightColor = game_->dirLight.lightColor;

	phong.dirLightDiffuseCoefficient = phongCoefficients.dirLightDiffuseCoefficient;
	phong.dirLightSpecularCoefficient_alpha = phongCoefficients.dirLightSpecularCoefficient_alpha;
	phong.dirLightAmbientCoefficient = phongCoefficients.dirLightAmbientCoefficient;

	phong.DSAIntensities = { game_->dirLight.diffuseIntensity, game_->dirLight.specularIntensity, game_->dirLight.ambientIntensity, 0.0f };

	game_->context->Map(constantPhongBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &phong, sizeof(phong));
	game_->context->Unmap(constantPhongBuffer, 0);

	UINT strides[] = { sizeof(DirectX::XMFLOAT4) * 4 };
	UINT offsets[] = { 0 };

	game_->context->RSSetState(material->rastState);

	game_->context->IASetInputLayout(material->layout);
	game_->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	game_->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
	game_->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
	game_->context->VSSetConstantBuffers(0, 1, &constantMvpBuffer);
	game_->context->PSSetConstantBuffers(1, 1, &constantPhongBuffer);
	game_->context->PSSetShaderResources(0, 1, &unlitDiffuseMaterial->textureView);
	game_->context->PSSetSamplers(0, 1, unlitDiffuseMaterial->pSampler.GetAddressOf());
	game_->context->VSSetShader(material->vertexShader, nullptr, 0);
	game_->context->PSSetShader(material->pixelShader, nullptr, 0);

	game_->context->OMSetRenderTargets(1, &game_->rtv, game_->pDSV.Get());
	game_->context->DrawIndexed(indicesLen, 0, 0);
}
