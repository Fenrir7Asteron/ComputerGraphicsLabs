#include "GBuffer.h"
#include <d3d11.h>

void GBuffer::Clear(ID3D11DeviceContext* context, float* color)
{
	context->ClearRenderTargetView(worldPos_DepthRTV, color);
	context->ClearRenderTargetView(normalRTV, color);
	context->ClearRenderTargetView(albedoRTV, color);
	context->ClearRenderTargetView(diffuseCoeffRTV, color);
	context->ClearRenderTargetView(specularCoeffRTV, color);
	context->ClearRenderTargetView(ambientCoeffRTV, color);
}