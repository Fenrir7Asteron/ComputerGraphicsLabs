#pragma once
#include "d3d11.h"
#include "SimpleMath.h"

class GameFramework;
class Camera;
class VertexPositionColor;

class DebugRenderSysImpl
{
	friend class GameFramework;
	
	GameFramework*		game = nullptr;
	Camera*		camera = nullptr;
	ID3D11Buffer* constBuf;

	DebugRenderSysImpl(GameFramework* inGame);


#pragma region Primitives
	std::vector<VertexPositionColor> lines;

	ID3D11VertexShader* vertexPrimShader;
	ID3D11PixelShader*	pixelPrimShader;
	ID3DBlob* vertexPrimCompResult;
	ID3DBlob* pixelPrimCompResult;

	ID3D11InputLayout*	layoutPrim;
	ID3D11Buffer*		verticesPrim;
	UINT bufPrimStride;

	ID3D11RasterizerState*		rastState;
	ID3D11DepthStencilState*	depthState;
	bool isPrimitivesDirty = false;

	const int MaxPointsCount = 1024 * 1024;
	int pointsCount = 0;
#pragma endregion Primitives

#pragma region Quads

	DirectX::SimpleMath::Matrix quadProjMatrix;
	
	struct QuadInfo
	{
		ID3D11ShaderResourceView* Srv;
		DirectX::SimpleMath::Matrix TransformMat;
	};

	std::vector<QuadInfo> quads;

	const UINT QuadMaxDrawCount = 100;


	ID3D11Buffer* quadBuf;
	UINT quadBindingStride;

	ID3D11PixelShader*	pixelQuadShader;
	ID3D11VertexShader*		vertexQuadShader;
	ID3DBlob* pixelQuadCompResult;
	ID3DBlob* vertexQuadCompResult;

	ID3D11InputLayout*	quadLayout;
	ID3D11SamplerState* quadSampler;

	ID3D11RasterizerState* quadRastState;

#pragma endregion Quads
	
protected:

	void InitPrimitives();
	void InitQuads();
	
	void DrawPrimitives();;
	void DrawQuads();

	void UpdateLinesBuffer();
	
public:
	~DebugRenderSysImpl();
	void SetCamera(Camera* inCamera);
	void Draw(float dTime);
	void Clear();

public:
	virtual void DrawBoundingBox(const DirectX::BoundingBox& box);
	virtual void DrawBoundingBox(const DirectX::BoundingBox& box, const DirectX::SimpleMath::Matrix& transform);
	virtual void DrawLine(const DirectX::SimpleMath::Vector3& pos0, const DirectX::SimpleMath::Vector3& pos1, const DirectX::SimpleMath::Color& color);
	virtual void DrawArrow(const DirectX::SimpleMath::Vector3& p0, const DirectX::SimpleMath::Vector3& p1, const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Vector3& n);
	virtual void DrawPoint(const DirectX::SimpleMath::Vector3& pos, const float& size);
	virtual void DrawCircle(const double& radius, const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Matrix& transform, int density);
	virtual void DrawSphere(const double& radius, const DirectX::SimpleMath::Color& color, const DirectX::SimpleMath::Matrix& transform, int density);
	virtual void DrawPlane(const DirectX::SimpleMath::Vector4& p, const DirectX::SimpleMath::Color& color, float sizeWidth, float sizeNormal, bool drawCenterCross);
	virtual void DrawGrid(float gridWidth, float cellWidth, const DirectX::SimpleMath::Color& color);

	virtual void DrawTextureOnScreen(ID3D11ShaderResourceView* tex, int x, int y, int width, int height, int zOrder);
};

