#include "ParticleSystem.h"
#include "GameFramework.h"
#include <d3dcompiler.h>
#include "magic_enum.hpp"
#include "MacrosParser.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using namespace magic_enum::bitwise_operators;

ParticleSystem::ParticleSystem(GameFramework* game,
	GameComponent* parent,
	Vector3 position,
	Quaternion rotation,
	Vector3 scale,
	Camera* camera) : GameComponent(game, parent, position, rotation, scale, nullptr)
{
	this->camera = camera;

	LoadShaders();
	CreateBuffers();

	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	game->device->CreateRasterizerState(&rastDesc, &rastState);

	auto blendStateDesc = D3D11_BLEND_DESC();
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

	game->device->CreateBlendState(&blendStateDesc, &blendState);

	auto depthDesc = D3D11_DEPTH_STENCIL_DESC();
	depthDesc.DepthEnable = true;
	depthDesc.StencilEnable = false;
	depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthDesc.StencilReadMask = 0x00;
	depthDesc.StencilWriteMask = 0x00;

	game->device->CreateDepthStencilState(&depthDesc, &depthState);
}

void ParticleSystem::GetGroupSize(int particlesCount, int& groupSizeX, int& groupSizeY)
{
	int numGroups = (particlesCount % THREAD_IN_GROUP_TOTAL != 0) ? ((particlesCount / THREAD_IN_GROUP_TOTAL) + 1) : (particlesCount / THREAD_IN_GROUP_TOTAL);
	double secondRoot = pow((double)numGroups, (double)(1.0f / 2.0f));
	secondRoot = ceil(secondRoot);
	groupSizeX = groupSizeY = (int)secondRoot;
}

void ParticleSystem::Update(float deltaTime)
{
	if (game_->inputDevice->IsKeyDown(Keys::H))
		deltaTime = 0.0f;

	int groupSizeX, groupSizeY;
	GetGroupSize(ParticlesCount, groupSizeX, groupSizeY);

	constData.World = GetWorldMatrix();
	constData.View = camera->GetViewMatrix();
	constData.Projection = camera->GetProjectionMatrix();
	constData.DeltatimeMaxparticlesGroupdim = { deltaTime, (float) ParticlesCount, (float) groupSizeY, 0.0f };

	game_->context->UpdateSubresource(constBuf, 0, nullptr, &constData, 0, 0);

	game_->context->CSSetConstantBuffers(0, 1, &constBuf);

	const UINT counterKeepValue = -1;
	const UINT counterZero = 0;

	game_->context->CSSetUnorderedAccessViews(0, 1, &uavSrc, &counterKeepValue);
	game_->context->CSSetUnorderedAccessViews(1, 1, &uavDst, &counterZero);

	game_->context->CSSetShader(ComputeShaders[ComputeFlags::SIMULATION], nullptr, 0);

	// simulate particles
	if (groupSizeX > 0)
		game_->context->Dispatch(groupSizeX, groupSizeY, 1);

	// inject new particles
	if (injectionCount > 0)
	{
		game_->context->CSSetConstantBuffers(0, 1, nullptr);

		int injSizeX, injSizeY;
		GetGroupSize(injectionCount, injSizeX, injSizeY);

		constData.DeltatimeMaxparticlesGroupdim = { deltaTime, (float)injectionCount, (float)injSizeY, 0.0f };

		game_->context->UpdateSubresource(constBuf, 0, nullptr, &constData, 0, 0);
		game_->context->CSSetConstantBuffers(0, 1, &constBuf);

		game_->context->UpdateSubresource(injectionBuf, 0, nullptr, &injectionParticles, 0, 0);

		game_->context->CSSetUnorderedAccessViews(0, 1, &injUav, &injectionCount);

		game_->context->CSSetShader(ComputeShaders[ComputeFlags::INJECTION], nullptr, 0);

		game_->context->Dispatch(injSizeX, injSizeY, 1);

		injectionCount = 0;
	}

	ID3D11UnorderedAccessView* nuPtr = nullptr;
	game_->context->CSSetUnorderedAccessViews(0, 1, &nuPtr, &counterZero);
	game_->context->CSSetUnorderedAccessViews(1, 1, &nuPtr, &counterZero);

	game_->context->CopyStructureCount(countBuf, 0, uavDst);

	D3D11_MAPPED_SUBRESOURCE subresource;
	game_->context->Map(countBuf, 0, D3D11_MAP_READ, 0, &subresource);

	UINT* data = reinterpret_cast<UINT*>(subresource.pData);
	ParticlesCount = data[0];

	game_->context->Unmap(countBuf, 0);

	SwapBuffers();
}

void ParticleSystem::GeometryPass()
{
	game_->context->ClearState();
	//game_->RestoreTargets();

	ID3D11RasterizerState* oldState = nullptr;
	game_->context->RSGetState(&oldState);
	game_->context->RSSetState(rastState);

	ID3D11BlendState* oldBlend = nullptr;
	UINT oldMask = 0;
	float oldBlendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	game_->context->OMGetBlendState(&oldBlend, oldBlendFactor, &oldMask);

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	game_->context->OMSetBlendState(blendState, blendFactor, 0xffffffff);

	ID3D11DepthStencilState* oldDepthState = nullptr;
	UINT oldStencilRef = 0;
	game_->context->OMGetDepthStencilState(&oldDepthState, &oldStencilRef);
	game_->context->OMGetDepthStencilState(&depthState, 0);

	game_->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	game_->context->VSSetShader(vertShader, nullptr, 0);
	game_->context->GSSetShader(geomShader, nullptr, 0);
	game_->context->PSSetShader(pixShader, nullptr, 0);

	game_->context->GSSetConstantBuffers(0, 1, &constBuf);
	game_->context->GSSetShaderResources(0, 1, &srvSrc);

	game_->context->OMSetRenderTargets(1, &game_->rtv, game_->pDSV.Get());
	game_->context->RSSetViewports(1, &game_->viewport);

	game_->context->Draw(ParticlesCount, 0);

	game_->context->OMSetBlendState(oldBlend, oldBlendFactor, oldMask);
	game_->context->RSSetState(oldState);
	game_->context->OMSetDepthStencilState(oldDepthState, oldStencilRef);

	if (oldState != NULL)
		oldState->Release();

	if (oldBlend != NULL)
		oldBlend->Release();

	if (oldDepthState != NULL)
		oldDepthState->Release();
}

void ParticleSystem::LoadShaders()
{
	ID3DBlob *vertexBC, *geometryBC, *pixelBC, *errorCode;
	const LPCWSTR shaderPath = L"./Shaders/ParticleSystemShader.hlsl";

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

	game_->device->CreateVertexShader(vertexBC->GetBufferPointer(), vertexBC->GetBufferSize(), nullptr, &vertShader);
	game_->device->CreateGeometryShader(geometryBC->GetBufferPointer(), geometryBC->GetBufferSize(), nullptr, &geomShader);
	game_->device->CreatePixelShader(pixelBC->GetBufferPointer(), pixelBC->GetBufferSize(), nullptr, &pixShader);

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

		D3DCompileFromFile(shaderPath,
			&macros[0],
			nullptr /*include*/,
			"CSMain",
			"cs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&computeBC,
			&errorCode);

		game_->device->CreateComputeShader(computeBC->GetBufferPointer(), computeBC->GetBufferSize(), nullptr, &ComputeShaders[flag]);
		computeBC->Release();
	}
}

void ParticleSystem::CreateBuffers()
{
	D3D11_BUFFER_DESC constBufDesc;

	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.CPUAccessFlags = 0;
	constBufDesc.MiscFlags = 0;
	constBufDesc.ByteWidth = sizeof(ConstData);

	game_->device->CreateBuffer(&constBufDesc, nullptr, &constBuf);

	D3D11_BUFFER_DESC bufDesc;
	bufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.StructureByteStride = sizeof(Particle);
	bufDesc.ByteWidth = MaxParticlesCount * sizeof(Particle);

	Particle* startParticles = new Particle[1];
	startParticles[0].Position = Vector4::Zero;

	Vector4 velocity = Vector4::Zero;
	velocity.y = 0.0f;

	startParticles[0].Velocity = velocity;
	startParticles[0].Color0 = { 1.0f, 0.0f, 0.0f, 1.0f };
	startParticles[0].Color1 = { 1.0f, 1.0f, 0.0f, 1.0f };
	startParticles[0].Size0Size1 = { 100.0f, 50.0f };
	startParticles[0].LifeTime = 10.0f;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &startParticles[0];
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	game_->device->CreateBuffer(&bufDesc, &data, &bufFirst);
	game_->device->CreateBuffer(&bufDesc, nullptr, &bufSecond);

	delete[] startParticles;


	game_->device->CreateShaderResourceView(bufFirst, nullptr, &srvFirst);
	game_->device->CreateShaderResourceView(bufSecond, nullptr, &srvSecond);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer = D3D11_BUFFER_UAV {
		0,
		MaxParticlesCount,
		D3D11_BUFFER_UAV_FLAG_APPEND
	};

	game_->device->CreateUnorderedAccessView(bufFirst, &uavDesc, &uavFirst);
	game_->device->CreateUnorderedAccessView(bufSecond, &uavDesc, &uavSecond);

	srvSrc = srvFirst;
	uavSrc = uavFirst;
	srvDst = srvSecond;
	uavDst = uavSecond;

	ID3D11UnorderedAccessView* nuPtr = nullptr;
	game_->context->CSSetUnorderedAccessViews(0, 1, &uavSrc, &StartParticlesCount);
	game_->context->CSSetUnorderedAccessViews(1, 1, &nuPtr, nullptr);

	D3D11_BUFFER_DESC countBufDesc;
	countBufDesc.BindFlags = 0;
	countBufDesc.Usage = D3D11_USAGE_STAGING;
	countBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	countBufDesc.MiscFlags = 0;
	countBufDesc.StructureByteStride = 0;
	countBufDesc.ByteWidth = 4;

	game_->device->CreateBuffer(&countBufDesc, nullptr, &countBuf);


	D3D11_BUFFER_DESC injBufDesc;
	injBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	injBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	injBufDesc.Usage = D3D11_USAGE_DEFAULT;
	injBufDesc.CPUAccessFlags = 0;
	injBufDesc.StructureByteStride = sizeof(Particle);
	injBufDesc.ByteWidth = MaxParticlesCount * sizeof(Particle);

	game_->device->CreateBuffer(&injBufDesc, nullptr, &injectionBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC injUavDesc;
	injUavDesc.Format = DXGI_FORMAT_UNKNOWN;
	injUavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	injUavDesc.Buffer = D3D11_BUFFER_UAV{
		0,
		MaxParticlesInjectionCount,
		D3D11_BUFFER_UAV_FLAG_APPEND
	};

	game_->device->CreateUnorderedAccessView(injectionBuf, &injUavDesc, &injUav);
}

void ParticleSystem::SwapBuffers()
{
	ID3D11ShaderResourceView* tempSrc = srvSrc;
	ID3D11UnorderedAccessView* tempUav = uavSrc;
	srvSrc = srvDst;
	uavSrc = uavDst;
	srvDst = tempSrc;
	uavDst = tempUav;
}
