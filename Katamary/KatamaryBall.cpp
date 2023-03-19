#define NOMINMAX
#define _USE_MATH_DEFINES
#include "KatamaryBall.h"
#include "GameFramework.h"
#include "ModelViewProjectionMatrices.h"
#include "UnlitDiffuseMaterial.h"
#include "DebugRenderSysImpl.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <PhongConstantData.h>
#include <Vertex.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX;
using namespace DirectX::SimpleMath;

KatamaryBall::KatamaryBall(GameFramework* game, float radius, int verticesNPerAxis, float moveSpeed, 
	const PhongCoefficients phongCoefficients,
	DirectX::SimpleMath::Matrix transform, Material* material, PhysicalLayer physicalLayer)
	: PhysicalSphereComponent(game, nullptr, physicalLayer, radius, transform, material)
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
			Vertex vert;
			vert.pos = { std::sin(theta) * std::cos(phi), std::cos(theta), std::sin(theta) * std::sin(phi), 1.0f };

			vert.col = { 1.0f, 1.0f, 1.0f, 1.0f };

			vert.n = vert.pos;
			vert.n.Normalize();

			float u = std::atan2(vert.n.x, vert.n.z) / (2 * M_PI) + 0.5;
			float v = vert.n.y * 0.5 + 0.5;
			vert.tex = { 1.0f - u, 1.0f - v, 0.0f, 0.0f };

			points.push_back(vert);

			phi += azimuthAngleStep;
		}

		theta = std::min(theta + inclinationAngleStep, (float)M_PI);
	}

	verticesLen = (int)points.size();

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

	indicesLen = (int)indices.size();

	this->scale = Vector3::One * radius;
	this->radius = radius;
	this->targetVolume = Volume();
	this->moveSpeed = moveSpeed;

	Move(Vector3::Up * radius);

	this->unlitDiffuseMaterial = dynamic_cast<UnlitDiffuseMaterial*>(material);

	boundingSphere.Center = positionOffset;
	boundingSphere.Radius = radius;


	D3D11_BUFFER_DESC vertexBufDesc = {};
	vertexBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufDesc.CPUAccessFlags = 0;
	vertexBufDesc.MiscFlags = 0;
	vertexBufDesc.StructureByteStride = 0;
	vertexBufDesc.ByteWidth = sizeof(Vertex) * verticesLen;

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

void KatamaryBall::Update(float deltaTime)
{
	float volume = Volume();
	if (volume + 0.001f < targetVolume)
	{
		float scaleDelta = std::min((float) std::pow((targetVolume - volume) / M_PI * (3.0f / 4.0f), 1.0f / 3.0f), deltaTime * 5.0f);
		IncreaseSize(scaleDelta);
	}

	Vector3 moveDir = Vector3::Zero;

	if (game_->inputDevice->IsKeyDown(Keys::W))	moveDir += Vector3::Forward;
	if (game_->inputDevice->IsKeyDown(Keys::A))	moveDir += Vector3::Left;
	if (game_->inputDevice->IsKeyDown(Keys::S))	moveDir += Vector3::Backward;
	if (game_->inputDevice->IsKeyDown(Keys::D))	moveDir += Vector3::Right;

	if (moveDir.LengthSquared() <= 0.001f)
		return;

	moveDir = Vector3::Transform(moveDir, game_->camera->rotation);
	moveDir.y = 0.0f;

	moveDir.Normalize();

	Vector3 rotationAxis = moveDir.Cross(Vector3::Up);

	float moveDelta = moveSpeed * deltaTime;

	Move(moveDir * moveDelta);
	Rotate(rotationAxis, -moveDelta / radius);

	for (auto attached : attachedObjects)
	{
		attached->Move(moveDir * moveDelta);
		attached->RotateAroundPoint(GetWorldMatrix().Translation(), rotationAxis, -moveDelta / radius);
	}

	GameComponent* other = game_->Intersects(this);
	if (other != nullptr)
	{
		float otherVolume = GetOtherObjectVolume(other);
		if (volume > otherVolume * 2.0f)
			AttachObject(other);
	}
}

void KatamaryBall::AttachObject(GameComponent* other)
{
	attachedObjects.push_back(other);
	other->enabled = false;

	targetVolume += GetOtherObjectVolume(other);
}

float KatamaryBall::GetOtherObjectVolume(GameComponent* other)
{
	PhysicalBoxComponent* otherBox = dynamic_cast<PhysicalBoxComponent*>(other);
	if (otherBox != nullptr)
	{
		Vector3 extents = otherBox->boundingBox.Extents;
		return extents.x * extents.y * extents.z * 8.0f;
	}

	return 0.0f;
}

float KatamaryBall::Volume()
{
	return (4.0f / 3.0f) * M_PI * std::pow(radius, 3);
}

void KatamaryBall::IncreaseSize(float sizeDelta)
{
	radius += sizeDelta;
	scale += Vector3::One * sizeDelta;
	positionOffset += Vector3::Up * sizeDelta;
	boundingSphere.Radius = radius;

	for (auto attached : attachedObjects)
	{
		Vector3 moveDir = attached->GetWorldMatrix().Translation() - GetWorldMatrix().Translation();
		moveDir.Normalize();

		attached->Move(moveDir * sizeDelta);
	}
}

void KatamaryBall::Draw()
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

	UINT strides[] = { sizeof(Vertex)};
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

	game_->debugRender->DrawSphere(boundingSphere.Radius, { 0.0f, 1.0f, 0.0f, 1.0f }, Matrix::CreateTranslation(boundingSphere.Center), 16);
}

