#include "GBuffer.h"
#include <d3d11.h>

GAMEFRAMEWORK_API void GBuffer::Clear(ID3D11DeviceContext* context, float* color)
{
	float color2[] = { 0.0f, 0.0f, 0.0f, 1000000.0f };
	context->ClearRenderTargetView(worldPos_DepthRTV, color2);

	context->ClearRenderTargetView(normalRTV, color);
	context->ClearRenderTargetView(albedoRTV, color);
	context->ClearRenderTargetView(diffuseCoeffRTV, color);
	context->ClearRenderTargetView(specularCoeffRTV, color);
	context->ClearRenderTargetView(ambientCoeffRTV, color);
}

void GBuffer::Release()
{
	worldPos_DepthMap->Release();
	normalMap->Release();
	albedoMap->Release();
	diffuseCoeffMap->Release();
	specularCoeffMap->Release();
	ambientCoeffMap->Release();

	worldPos_DepthRTV->Release();
	normalRTV->Release();
	albedoRTV->Release();
	diffuseCoeffRTV->Release();
	specularCoeffRTV->Release();
	ambientCoeffRTV->Release();

	worldPos_DepthSRV->Release();
	normalSRV->Release();
	albedoSRV->Release();
	diffuseCoeffSRV->Release();
	specularCoeffSRV->Release();
	ambientCoeffSRV->Release();
}
