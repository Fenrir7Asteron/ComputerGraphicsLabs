#pragma once
class ID3D11Texture2D;
class ID3D11RenderTargetView;
class ID3D11ShaderResourceView;
class ID3D11DeviceContext;

class GBuffer
{
public:
	ID3D11Texture2D* worldPos_DepthMap;
	ID3D11Texture2D* normalMap;
	ID3D11Texture2D* albedoMap;
	ID3D11Texture2D* diffuseCoeffMap;
	ID3D11Texture2D* specularCoeffMap;
	ID3D11Texture2D* ambientCoeffMap;

	ID3D11RenderTargetView* worldPos_DepthRTV;
	ID3D11RenderTargetView* normalRTV;
	ID3D11RenderTargetView* albedoRTV;
	ID3D11RenderTargetView* diffuseCoeffRTV;
	ID3D11RenderTargetView* specularCoeffRTV;
	ID3D11RenderTargetView* ambientCoeffRTV;

	ID3D11ShaderResourceView* worldPos_DepthSRV;
	ID3D11ShaderResourceView* normalSRV;
	ID3D11ShaderResourceView* albedoSRV;
	ID3D11ShaderResourceView* diffuseCoeffSRV;
	ID3D11ShaderResourceView* specularCoeffSRV;
	ID3D11ShaderResourceView* ambientCoeffSRV;

	void Clear(ID3D11DeviceContext* context, float* color);
};
