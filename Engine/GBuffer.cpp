#include "GBuffer.h"
#include <d3d11.h>

void GBuffer::Clear(ID3D11DeviceContext* context, float* color)
{
	float color2[] = { 0.0f, 0.0f, 0.0f, 1000000.0f };
	context->ClearRenderTargetView(worldPos_DepthRTV, color2);

	context->ClearRenderTargetView(normalRTV, color);
	context->ClearRenderTargetView(albedoRTV, color);
	context->ClearRenderTargetView(diffuseCoeffRTV, color);
	context->ClearRenderTargetView(specularCoeffRTV, color);
	context->ClearRenderTargetView(ambientCoeffRTV, color);
}