#include "ParticleSystemSorted.h"
#include "GameFramework.h"
#include <d3dcompiler.h>
#include "magic_enum.hpp"
#include "MacrosParser.h"
#include "Component/GameObject.h"
#include "Camera/Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using namespace magic_enum::bitwise_operators;

GAMEFRAMEWORK_API ParticleSystemSorted::ParticleSystemSorted(GameObject* gameObject, Camera* camera)
	: GameComponent(gameObject)
{
	this->camera = camera;

	LoadShaders();
	CreateBuffers();

	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	GameFramework::Instance->device->CreateRasterizerState(&rastDesc, &rastState);

	auto blendStateDesc = D3D11_BLEND_DESC();
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	GameFramework::Instance->device->CreateBlendState(&blendStateDesc, &blendState);

	auto depthDesc = D3D11_DEPTH_STENCIL_DESC();
	depthDesc.DepthEnable = true;
	depthDesc.StencilEnable = false;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	//depthDesc.StencilWriteMask = 0x00;

	GameFramework::Instance->device->CreateDepthStencilState(&depthDesc, &depthState);


	sort = new BitonicSort();
}

GAMEFRAMEWORK_API void ParticleSystemSorted::GetGroupSize(int particlesCount, int& groupSizeX, int& groupSizeY)
{
	if (particlesCount <= 0)
	{
		groupSizeX = 0;
		groupSizeY = 0;
	}
	else
	{
		groupSizeX = (particlesCount - 1) / ThreadInGroupTotal + 1;
		groupSizeY = 1;
	}
}

GAMEFRAMEWORK_API void ParticleSystemSorted::LoadShaders()
{
	auto device = GameFramework::Instance->device;

	ID3DBlob* vertexBC, * geometryBC, * pixelBC, * errorCode;
	const LPCWSTR shaderPath = L"./Shaders/ParticleSystemSortedShader.hlsl";

	D3DCompileFromFile(shaderPath,
		nullptr,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorCode);

	D3DCompileFromFile(shaderPath,
		nullptr,
		nullptr /*include*/,
		"GSMain",
		"gs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&geometryBC,
		&errorCode);

	D3DCompileFromFile(shaderPath,
		nullptr,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelBC,
		&errorCode);

	device->CreateVertexShader(vertexBC->GetBufferPointer(), vertexBC->GetBufferSize(), nullptr, &vertShader);
	device->CreateGeometryShader(geometryBC->GetBufferPointer(), geometryBC->GetBufferSize(), nullptr, &geomShader);
	device->CreatePixelShader(pixelBC->GetBufferPointer(), pixelBC->GetBufferSize(), nullptr, &pixShader);

	vertexBC->Release();
	geometryBC->Release();
	pixelBC->Release();

	std::vector<ComputeFlags> flags =
	{
		ComputeFlags::INJECTION,
		ComputeFlags::SIMULATION,
		ComputeFlags::SIMULATION | ComputeFlags::ADD_GRAVITY,
	};

	for (auto& flag : flags)
	{
		auto macros = MacrosParser::GetMacros(flag);

		ID3DBlob* computeBC = nullptr;

		HRESULT res = D3DCompileFromFile(shaderPath,
			&macros[0],
			nullptr /*include*/,
			"CSMain",
			"cs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&computeBC,
			&errorCode);

		GameFramework::Instance->CheckShaderCreationSuccess(res, errorCode, shaderPath);

		device->CreateComputeShader(computeBC->GetBufferPointer(), computeBC->GetBufferSize(), nullptr, &ComputeShaders[flag]);
		computeBC->Release();
	}
}

GAMEFRAMEWORK_API void ParticleSystemSorted::CreateBuffers()
{
	auto device = GameFramework::Instance->device;
	auto* context = GameFramework::Instance->context;

	D3D11_BUFFER_DESC constBufDesc;

	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.ByteWidth = sizeof(ConstData);

	device->CreateBuffer(&constBufDesc, nullptr, &constBuf);

	D3D11_BUFFER_DESC poolBufDesc;
	poolBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	poolBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	poolBufDesc.Usage = D3D11_USAGE_DEFAULT;
	poolBufDesc.CPUAccessFlags = 0;
	poolBufDesc.StructureByteStride = sizeof(Particle);
	poolBufDesc.ByteWidth = MaxParticlesCount * sizeof(Particle);

	device->CreateBuffer(&poolBufDesc, nullptr, &particlesPool);

	device->CreateShaderResourceView(particlesPool, nullptr, &srvPool);
	device->CreateUnorderedAccessView(particlesPool, nullptr, &uavPool);


	D3D11_BUFFER_DESC sortBufDesc;
	sortBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	sortBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	sortBufDesc.Usage = D3D11_USAGE_DEFAULT;
	sortBufDesc.CPUAccessFlags = 0;
	sortBufDesc.StructureByteStride = sizeof(DirectX::SimpleMath::Vector2);
	sortBufDesc.ByteWidth = MaxParticlesCount * sizeof(DirectX::SimpleMath::Vector2);

	device->CreateBuffer(&sortBufDesc, nullptr, &sortBuffer);

	auto indices = new UINT[MaxParticlesCount];
	for (UINT i = 0; i < MaxParticlesCount; i++)
	{
		indices[i] = MaxParticlesCount - 1 - i;
	}

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &indices[0];
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;


	D3D11_BUFFER_DESC deadBufDesc;
	deadBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	deadBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	deadBufDesc.Usage = D3D11_USAGE_DEFAULT;
	deadBufDesc.CPUAccessFlags = 0;
	deadBufDesc.StructureByteStride = sizeof(UINT);
	deadBufDesc.ByteWidth = MaxParticlesCount * sizeof(UINT);

	device->CreateBuffer(&deadBufDesc, &data, &deadBuf);
	delete[] indices;

	device->CreateShaderResourceView(sortBuffer, nullptr, &srvSorted);


	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer = D3D11_BUFFER_UAV{
		0,
		MaxParticlesCount,
		D3D11_BUFFER_UAV_FLAG_APPEND
	};

	device->CreateUnorderedAccessView(sortBuffer, &uavDesc, &uavSorted);
	device->CreateUnorderedAccessView(deadBuf, &uavDesc, &uavDead);


	ID3D11UnorderedAccessView* nuPtr = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &uavPool, nullptr);
	context->CSSetUnorderedAccessViews(0, 1, &nuPtr, nullptr);

	D3D11_BUFFER_DESC countBufDesc;
	countBufDesc.BindFlags = 0;
	countBufDesc.Usage = D3D11_USAGE_STAGING;
	countBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	countBufDesc.MiscFlags = 0;
	countBufDesc.StructureByteStride = 0;
	countBufDesc.ByteWidth = 4;

	device->CreateBuffer(&countBufDesc, nullptr, &countBuf);


	D3D11_BUFFER_DESC injBufDesc;
	injBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	injBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	injBufDesc.Usage = D3D11_USAGE_DEFAULT;
	injBufDesc.CPUAccessFlags = 0;
	injBufDesc.StructureByteStride = sizeof(Particle);
	injBufDesc.ByteWidth = MaxParticlesInjectionCount * sizeof(Particle);

	device->CreateBuffer(&injBufDesc, nullptr, &injectionBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC injUavDesc;
	injUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	injUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	injUavDesc.Buffer = D3D11_BUFFER_UAV{
		0,
		MaxParticlesInjectionCount,
		D3D11_BUFFER_UAV_FLAG_APPEND
	};

	device->CreateUnorderedAccessView(injectionBuf, &injUavDesc, &injUav);
}

GAMEFRAMEWORK_API void ParticleSystemSorted::DrawShadowMap()
{
}

GAMEFRAMEWORK_API void ParticleSystemSorted::Update(float deltaTime)
{
	auto* context = GameFramework::Instance->context;

	//sort->Sort(uavSorted, srvSorted);

	if (GameFramework::Instance->inputDevice->IsKeyDown(Keys::Space))
		deltaTime = 0.0f;

	int groupSizeX, groupSizeY;
	GetGroupSize(ParticlesCount, groupSizeX, groupSizeY);

	constData.World = m_gameObject->GetTransform()->GetWorldMatrix();
	constData.View = camera->GetViewMatrix();
	constData.Projection = camera->GetProjectionMatrix();
	constData.DeltaTimeMaxParticlesGroupdim = { deltaTime, (float)ParticlesCount, (float)groupSizeX, 0.0f };

	context->UpdateSubresource(constBuf, 0, nullptr, &constData, 0, 0);
	context->CSSetConstantBuffers(0, 1, &constBuf);

	ID3D11ShaderResourceView* SRVs[] = { GameFramework::Instance->gBuffer.worldPos_DepthSRV, GameFramework::Instance->gBuffer.normalSRV };
	context->CSSetShaderResources(1, 2, SRVs);

	const UINT counterKeepValue = -1;
	const UINT counterZero = 0;

	context->CSSetUnorderedAccessViews(0, 1, &uavPool, &counterKeepValue);
	context->CSSetUnorderedAccessViews(1, 1, &uavSorted, &counterKeepValue);
	context->CSSetUnorderedAccessViews(2, 1, &uavDead, &counterKeepValue);

	context->CSSetShader(ComputeShaders[ComputeFlags::SIMULATION | ComputeFlags::ADD_GRAVITY], nullptr, 0);

	// simulate particles
	if (groupSizeX > 0)
		context->Dispatch(groupSizeX, groupSizeY, 1);

	// inject new particles
	if (injectionCount > 0)
	{
		//game_->context->CSSetConstantBuffers(0, 1, nullptr);

		int injSizeX, injSizeY;
		GetGroupSize(injectionCount, injSizeX, injSizeY);

		constData.DeltaTimeMaxParticlesGroupdim = { deltaTime, (float)injectionCount, (float)injSizeX, 0.0f };

		context->UpdateSubresource(constBuf, 0, nullptr, &constData, 0, 0);
		context->CSSetConstantBuffers(0, 1, &constBuf);

		context->UpdateSubresource(injectionBuf, 0, nullptr, injectionParticles, 0, 0);

		context->CSSetUnorderedAccessViews(3, 1, &injUav, &injectionCount);

		context->CSSetShader(ComputeShaders[ComputeFlags::INJECTION], nullptr, 0);

		context->Dispatch(injSizeX, injSizeY, 1);

		injectionCount = 0;
	}

	ID3D11UnorderedAccessView* nuPtr = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &nuPtr, &counterZero);
	context->CSSetUnorderedAccessViews(1, 1, &nuPtr, &counterZero);
	context->CSSetUnorderedAccessViews(2, 1, &nuPtr, &counterZero);


	context->CopyStructureCount(countBuf, 0, uavPool);

	D3D11_MAPPED_SUBRESOURCE subresource;
	context->Map(countBuf, 0, D3D11_MAP_READ, 0, &subresource);

	UINT* data = reinterpret_cast<UINT*>(subresource.pData);

	ParticlesCount = data[0];

	context->Unmap(countBuf, 0);


	context->CopyStructureCount(countBuf, 0, uavDead);

	context->Map(countBuf, 0, D3D11_MAP_READ, 0, &subresource);

	data = reinterpret_cast<UINT*>(subresource.pData);

	ParticlesDeadCount = data[0];

	context->Unmap(countBuf, 0);
}

GAMEFRAMEWORK_API void ParticleSystemSorted::Destroy()
{
}

GAMEFRAMEWORK_API void ParticleSystemSorted::GeometryPass()
{
	auto* context = GameFramework::Instance->context;

	context->ClearState();

	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	context->RSSetState(rastState);

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	context->OMSetDepthStencilState(depthState, 0);

	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	context->VSSetShader(vertShader, nullptr, 0);
	context->GSSetShader(geomShader, nullptr, 0);
	context->PSSetShader(pixShader, nullptr, 0);

	context->GSSetConstantBuffers(0, 1, &constBuf);
	context->GSSetShaderResources(0, 1, &srvPool);

	context->OMSetRenderTargets(1, &GameFramework::Instance->rtv, GameFramework::Instance->pDSV.Get());
	context->RSSetViewports(1, &GameFramework::Instance->viewport);

	context->Draw(ParticlesCount, 0);
}

GAMEFRAMEWORK_API void ParticleSystemSorted::AddParticle(const Particle& p)
{
	if (GameFramework::Instance->inputDevice->IsKeyDown(Keys::Space))
		return;

	if (injectionCount < MaxParticlesInjectionCount && ParticlesCount + injectionCount < MaxParticlesCount)
	{
		injectionParticles[injectionCount] = p;
		injectionCount++;
	}
}