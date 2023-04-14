#include "pch.h"
#include "DebugRenderSysImpl.h"
#include "Camera.h"
#include "VertexPositionColor.h"
#include "GameFramework.h"
#include <d3dcompiler.h>

using namespace DirectX::SimpleMath;

DebugRenderSysImpl::DebugRenderSysImpl(GameFramework* inGame) : game(inGame)
{
	D3D11_BUFFER_DESC constDesc = {};
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.CPUAccessFlags = 0;
	constDesc.MiscFlags = 0;
	constDesc.Usage = D3D11_USAGE_DEFAULT;
	constDesc.ByteWidth = sizeof(Matrix);

	game->device->CreateBuffer(&constDesc, nullptr, &constBuf);
	
	InitPrimitives();
	//InitQuads();
}


void DebugRenderSysImpl::InitPrimitives()
{
	HRESULT res;
	ID3DBlob* errorCode = nullptr;
	
	res = D3DCompileFromFile(L"./Shaders/Simple.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
		0,
		&vertexPrimCompResult,
		&errorCode);

	if(errorCode) errorCode->Release();
	
	res = D3DCompileFromFile(L"./Shaders/Simple.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR,
		0,
		&pixelPrimCompResult,
		&errorCode);

	if (errorCode) errorCode->Release();

	game->device->CreateVertexShader(vertexPrimCompResult->GetBufferPointer(), vertexPrimCompResult->GetBufferSize(), nullptr, &vertexPrimShader);
	game->device->CreatePixelShader(pixelPrimCompResult->GetBufferPointer(), pixelPrimCompResult->GetBufferSize(), nullptr, &pixelPrimShader);

	layoutPrim = VertexPositionColor::GetLayout(vertexPrimCompResult);


	D3D11_BUFFER_DESC bufDesc = {};
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufDesc.MiscFlags = 0;
	bufDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufDesc.ByteWidth = MaxPointsCount * VertexPositionColor::Stride;

	game->device->CreateBuffer(&bufDesc, nullptr, &verticesPrim);
	bufPrimStride = VertexPositionColor::Stride;
	
	
	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_WIREFRAME;

	res = game->device->CreateRasterizerState(&rastDesc, &rastState);

	
	D3D11_DEPTH_STENCIL_DESC depthDesc = {};
	depthDesc.DepthEnable = false;
	
	game->device->CreateDepthStencilState(&depthDesc, &depthState);

	lines.reserve(100);
}


void DebugRenderSysImpl::InitQuads()
{
	quadProjMatrix = Matrix::CreateOrthographicOffCenter(0, static_cast<float>(game->displayWin->windowWidth), static_cast<float>(game->displayWin->windowHeight), 0, 0.1f, 1000.0f);

	ID3DBlob* errorCode = nullptr;
	
	D3DCompileFromFile(L"Shaders/TexturedShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &vertexQuadCompResult, &errorCode);
	game->device->CreateVertexShader(vertexQuadCompResult->GetBufferPointer(), vertexQuadCompResult->GetBufferSize(), nullptr, &vertexQuadShader);

	if (errorCode)errorCode->Release();
	
	D3DCompileFromFile(L"Shaders/TexturedShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, &pixelQuadCompResult, &errorCode);
	game->device->CreatePixelShader(pixelQuadCompResult->GetBufferPointer(), pixelQuadCompResult->GetBufferSize(), nullptr, &pixelQuadShader);

	if (errorCode)errorCode->Release();

	quadLayout = VertexPositionColor::GetLayout(vertexQuadCompResult);
	quadBindingStride = VertexPositionColor::Stride;

	quads.reserve(10);

	auto points = new Vector4[8] {
		Vector4(1, 1, 0.0f, 1.0f), Vector4(1.0f, 1.0f, 0.0f, 0.0f),
		Vector4(0, 1, 0.0f, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f),
		Vector4(1, 0, 0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 0.0f),
		Vector4(0, 0, 0.0f, 1.0f), Vector4(0.0f, 0.0f, 0.0f, 0.0f),
	};

	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufDesc.CPUAccessFlags = 0;
		bufDesc.MiscFlags = 0;
		bufDesc.Usage = D3D11_USAGE_DEFAULT;
		bufDesc.ByteWidth = sizeof(float) * 4 * 8;
	}

	D3D11_SUBRESOURCE_DATA subData = {};
	subData.pSysMem = points;
	
	game->device->CreateBuffer(&bufDesc, &subData, &quadBuf);

	delete[] points;

	float borderCol[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	
	D3D11_SAMPLER_DESC samplDesc = {};
	{
		samplDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplDesc.BorderColor[0] = 1.0f;
		samplDesc.BorderColor[1] = 0.0f;
		samplDesc.BorderColor[2] = 0.0f;
		samplDesc.BorderColor[3] = 1.0f;
		samplDesc.MaxLOD = static_cast<float>(INT_MAX);
	}
	game->device->CreateSamplerState(&samplDesc, &quadSampler);

	D3D11_RASTERIZER_DESC rastDesc = {};
	{
		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_SOLID;
	}

	game->device->CreateRasterizerState(&rastDesc, &quadRastState);
}


void DebugRenderSysImpl::DrawPrimitives()
{
	if (isPrimitivesDirty) {
		UpdateLinesBuffer();
		pointsCount = lines.size();

		isPrimitivesDirty = false;
	}

	auto mat = camera->GetViewMatrix() * camera->GetProjectionMatrix();

	game->context->UpdateSubresource(constBuf, 0, nullptr, &mat, 0, 0);

	game->context->OMSetDepthStencilState(depthState, 0);
	game->context->RSSetState(rastState);

	game->context->VSSetShader(vertexPrimShader, nullptr, 0);
	game->context->PSSetShader(pixelPrimShader, nullptr, 0);

	game->context->IASetInputLayout(layoutPrim);
	game->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	const UINT offset = 0;
	game->context->IASetVertexBuffers(0, 1, &verticesPrim, &bufPrimStride, &offset);

	game->context->VSSetConstantBuffers(0, 1, &constBuf);

	game->context->Draw(pointsCount, 0);
}


void DebugRenderSysImpl::DrawQuads()
{
	if (quads.empty()) return;

	game->context->OMSetDepthStencilState(depthState, 0);
	game->context->RSSetState(quadRastState);
	
	game->context->VSSetShader(vertexQuadShader, nullptr, 0);
	game->context->PSSetShader(pixelQuadShader, nullptr, 0);

	game->context->IASetInputLayout(quadLayout);
	game->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	const UINT offset = 0;
	game->context->IASetVertexBuffers(0, 1, &quadBuf, &quadBindingStride, &offset);

	game->context->VSSetConstantBuffers(0, 1, &constBuf);

	for(auto& quad : quads) {
		auto mat = quad.TransformMat * quadProjMatrix;
		game->context->UpdateSubresource(constBuf, 0, nullptr, &mat, 0, 0);

		game->context->PSSetShaderResources(0, 1, &quad.Srv);
		game->context->PSSetSamplers(0, 1, &quadSampler);

		game->context->Draw(4, 0);
	}
}

void DebugRenderSysImpl::UpdateLinesBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	game->context->Map(verticesPrim, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, lines.data(), lines.size() * VertexPositionColor::Stride);

	game->context->Unmap(verticesPrim, 0);
}


DebugRenderSysImpl::~DebugRenderSysImpl()
{
}


void DebugRenderSysImpl::SetCamera(Camera* inCamera)
{
	camera = inCamera;
}


void DebugRenderSysImpl::Draw(float dTime)
{
	if (camera == nullptr) return;

	//game->context->ClearState();
	//game->RestoreTargets();

	D3D11_VIEWPORT viewport = {
		0,
		0,
		static_cast<float>(game->displayWin->windowWidth),
		static_cast<float>(game->displayWin->windowHeight),
		0,
		1
	};
	
	game->context->RSSetViewports(1, &viewport);

	DrawPrimitives();
	DrawQuads();
}


void DebugRenderSysImpl::Clear()
{
	lines.clear();
	quads.clear();
}


void DebugRenderSysImpl::DrawBoundingBox(const DirectX::BoundingBox& box)
{
	Vector3 corners[8];
	
	box.GetCorners(&corners[0]);
	
	DrawLine(corners[0], corners[1], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[1], corners[2], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[2], corners[3], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[3], corners[0], Color(0.0f, 1.0f, 0.0f, 1.0f));

	DrawLine(corners[4], corners[5], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[5], corners[6], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[6], corners[7], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[7], corners[4], Color(0.0f, 1.0f, 1.0f, 1.0f));

	DrawLine(corners[0], corners[4], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[1], corners[5], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[2], corners[6], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[3], corners[7], Color(0.0f, 0.0f, 1.0f, 1.0f));
}


void DebugRenderSysImpl::DrawBoundingBox(const DirectX::BoundingBox& box, const DirectX::SimpleMath::Matrix& transform)
{
	Vector3 corners[8];
	box.GetCorners(&corners[0]);

	for(auto& corner : corners) {
		corner = Vector3::Transform(corner, transform);
	}

	DrawLine(corners[0], corners[1], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[1], corners[2], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[2], corners[3], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[3], corners[0], Color(0.0f, 1.0f, 0.0f, 1.0f));

	DrawLine(corners[4], corners[5], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[5], corners[6], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[6], corners[7], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[7], corners[4], Color(0.0f, 1.0f, 1.0f, 1.0f));

	DrawLine(corners[0], corners[4], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[1], corners[5], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[2], corners[6], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[3], corners[7], Color(0.0f, 0.0f, 1.0f, 1.0f));
}

void DebugRenderSysImpl::DrawOrientedBoundingBox(const DirectX::BoundingOrientedBox& box, const DirectX::SimpleMath::Matrix& transform)
{
	Vector3 corners[8];
	box.GetCorners(&corners[0]);

	for (auto& corner : corners) {
		corner = Vector3::Transform(corner, transform);
	}

	DrawLine(corners[0], corners[1], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[1], corners[2], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[2], corners[3], Color(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(corners[3], corners[0], Color(0.0f, 1.0f, 0.0f, 1.0f));

	DrawLine(corners[4], corners[5], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[5], corners[6], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[6], corners[7], Color(0.0f, 1.0f, 1.0f, 1.0f));
	DrawLine(corners[7], corners[4], Color(0.0f, 1.0f, 1.0f, 1.0f));

	DrawLine(corners[0], corners[4], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[1], corners[5], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[2], corners[6], Color(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(corners[3], corners[7], Color(0.0f, 0.0f, 1.0f, 1.0f));
}

void DebugRenderSysImpl::DrawLine(const DirectX::SimpleMath::Vector3& pos0, const DirectX::SimpleMath::Vector3& pos1,
	const DirectX::SimpleMath::Color& color)
{
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos0.x, pos0.y, pos0.z, 1.0f),
			color.ToVector4()
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos1.x, pos1.y, pos1.z, 1.0f),
			color.ToVector4()
		});

	isPrimitivesDirty = true;
}


void DebugRenderSysImpl::DrawArrow(const DirectX::SimpleMath::Vector3& p0, const DirectX::SimpleMath::Vector3& p1,
	const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Vector3& n)
{
	DrawLine(p0, p1, color);

	auto a = Vector3::Lerp(p0, p1, 0.85f);

	auto diff = p1 - p0;
	auto side = n.Cross(diff) * 0.05f;

	DrawLine(a + side, p1, color);
	DrawLine(a - side, p1, color);
}


void DebugRenderSysImpl::DrawPoint(const DirectX::SimpleMath::Vector3& pos, const float& size)
{
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x + size, pos.y, pos.z, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x - size, pos.y, pos.z, 1.0f),
			Vector4(1.0f, 0.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y + size, pos.z, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y - size, pos.z, 1.0f),
			Vector4(0.0f, 1.0f, 0.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y, pos.z + size, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f)
		});
	lines.emplace_back(VertexPositionColor
		{
			Vector4(pos.x, pos.y, pos.z - size, 1.0f),
			Vector4(0.0f, 0.0f, 1.0f, 1.0f)
		});

	isPrimitivesDirty = true;
}


void DebugRenderSysImpl::DrawCircle(const double& radius, const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Matrix& transform, int density)
{
	double angleStep = DirectX::XM_PI * 2 / density;

	for (int i = 0; i < density; i++)
	{
		auto point0X = radius * cos(angleStep * i);
		auto point0Y = radius * sin(angleStep * i);

		auto point1X = radius * cos(angleStep * (i + 1));
		auto point1Y = radius * sin(angleStep * (i + 1));

		auto p0 = Vector3::Transform(Vector3(static_cast<float>(point0X), static_cast<float>(point0Y), 0), transform);
		auto p1 = Vector3::Transform(Vector3(static_cast<float>(point1X), static_cast<float>(point1Y), 0), transform);

		DrawLine(p0, p1, color);
	}
}


void DebugRenderSysImpl::DrawSphere(const double& radius, const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Matrix& transform, int density)
{
	DrawCircle(radius, color, transform, density);
	DrawCircle(radius, color, Matrix::CreateRotationX(DirectX::XM_PIDIV2) * transform, density);
	DrawCircle(radius, color, Matrix::CreateRotationY(DirectX::XM_PIDIV2) * transform, density);
}

void DebugRenderSysImpl::DrawPlane(const DirectX::SimpleMath::Vector4& p, const DirectX::SimpleMath::Color& color, float sizeWidth, float sizeNormal, bool drawCenterCross)
{
	auto dir = Vector3(p.x, p.y, p.z);
	if (dir.Length() == 0.0f) return;
	dir.Normalize();
	
	auto up = Vector3(0, 0, 1);
	auto right = dir.Cross(up);
	if(right.Length() < 0.01f) {
		up = Vector3(0, 1, 0);
		right = dir.Cross(up);
	}
	right.Normalize();

	up = right.Cross(dir);
	
	auto pos = -dir * p.w;

	auto leftPoint	= pos - right * sizeWidth;
	auto rightPoint	= pos + right * sizeWidth;
	auto downPoint	= pos - up * sizeWidth;
	auto upPoint		= pos + up * sizeWidth;

	DrawLine(leftPoint + up * sizeWidth, rightPoint + up * sizeWidth, color);
	DrawLine(leftPoint - up * sizeWidth, rightPoint - up * sizeWidth, color);
	DrawLine(downPoint - right * sizeWidth, upPoint - right * sizeWidth, color);
	DrawLine(downPoint + right * sizeWidth, upPoint + right * sizeWidth, color);
	
	
	if(drawCenterCross) {
		DrawLine(leftPoint, rightPoint, color);
		DrawLine(downPoint, upPoint, color);
	}
	
	DrawPoint(pos, 0.5f);
	DrawArrow(pos, pos + dir*sizeNormal, color, right);
}

void DebugRenderSysImpl::DrawGrid(float gridWidth, float cellWidth, const DirectX::SimpleMath::Color& color)
{
	int sideLineCount = gridWidth / cellWidth;

	if (sideLineCount <= 0)
		return;

	int half = sideLineCount / 2;
	Vector3 curr = Vector3::Right * -gridWidth * 0.5f;

	for (int i = -half; i <= half; ++i) 
	{
		DrawLine(curr + Vector3::Backward * gridWidth * 0.5f, curr + Vector3::Forward * gridWidth * 0.5f, color);
		curr += Vector3::Right * cellWidth;
	}

	curr = Vector3::Forward * -gridWidth * 0.5f;

	for (int i = -half; i <= half; ++i)
	{
		DrawLine(curr + Vector3::Left * gridWidth * 0.5f, curr + Vector3::Right * gridWidth * 0.5f, color);
		curr += Vector3::Forward * cellWidth;
	}
}


void DebugRenderSysImpl::DrawTextureOnScreen(ID3D11ShaderResourceView* tex, int x, int y, int width, int height, int zOrder)
{
	if (quads.size() >= QuadMaxDrawCount) return;

	QuadInfo quad = {};
	quad.Srv = tex;
	quad.TransformMat = Matrix::CreateScale(static_cast<float>(width), static_cast<float>(height), 1.0f)
						* Matrix::CreateTranslation(static_cast<float>(x), static_cast<float>(y), static_cast<float>(zOrder));
	
	quads.emplace_back(quad);
}

