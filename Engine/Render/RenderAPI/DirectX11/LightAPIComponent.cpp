#include "LightAPIComponent.h"
#include "ShaderAPIUtils.h"
#include "DirectX11RenderAPI.h"
#include "MacrosParser.h"
#include "Model.h"
#include "Camera/Camera.h"
#include "Component/GameObject.h"
#include "Component/Render/Light/LightConstantData.h"
#include "Component/Render/Light/DirectionalLight.h"
#include "Component/Render/Light/PointLight.h"
#include "Render/Debug/DebugRenderSysImpl.h"
#include <d3dcommon.h>
#include <d3dcompiler.h>


using namespace DirectX::SimpleMath;

namespace RenderAPI::DirectX11
{
	GAMEFRAMEWORK_API LightAPIComponent::LightAPIComponent()
	{
		auto device = DirectX11RenderAPI::Instance->device;

		// Create lighting pass resources shaders
		ID3DBlob* vertexBC = nullptr;
		ID3DBlob* lightingSignature = nullptr;
		ID3DBlob* errorVertexCode = nullptr;
		HRESULT res;

		const std::wstring lightingPassShaderPath = L"./Shaders/LightingShader.hlsl";

		for (auto flag : vsLightingFlags)
		{
			auto macros = MacrosParser::GetMacros(flag);

			res = D3DCompileFromFile(lightingPassShaderPath.c_str(),
				&macros[0],
				nullptr /*include*/,
				"VSMain",
				"vs_5_0",
				D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
				0,
				&vertexBC,
				&errorVertexCode);

			ShaderAPIUtils::CheckShaderCreationSuccess(res, errorVertexCode, lightingPassShaderPath);

			res = device->CreateVertexShader(
				vertexBC->GetBufferPointer(),
				vertexBC->GetBufferSize(),
				nullptr, &lightingVertexShaders[flag]);

			if (flag == VertexLightingShaderFlags::NONE)
			{
				lightingSignature = vertexBC;
			}
		}

		ID3DBlob* pixelBC = nullptr;
		ID3DBlob* errorPixelCode = nullptr;


		for (auto flag : psLightingFlags)
		{
			auto macros = MacrosParser::GetMacros(flag);

			res = D3DCompileFromFile(lightingPassShaderPath.c_str(),
				&macros[0] /*macros*/,
				nullptr /*include*/,
				"PSMain",
				"ps_5_0",
				D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
				0,
				&pixelBC,
				&errorPixelCode);

			ShaderAPIUtils::CheckShaderCreationSuccess(res, errorPixelCode, lightingPassShaderPath);

			res = device->CreatePixelShader(
				pixelBC->GetBufferPointer(),
				pixelBC->GetBufferSize(),
				nullptr, &lightingPixelShaders[flag]);
		}

		if (FAILED(res))
		{
			return;
		}

		D3D11_INPUT_ELEMENT_DESC inputElements[] = {
			D3D11_INPUT_ELEMENT_DESC {
				"POSITION",
				0,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				0,
				0,
				D3D11_INPUT_PER_VERTEX_DATA,
				0}
		};

		device->CreateInputLayout(
			inputElements,
			1,
			lightingSignature->GetBufferPointer(),
			lightingSignature->GetBufferSize(),
			&lightingLayout);


		D3D11_BUFFER_DESC constantLightBufDesc = {};
		constantLightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantLightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantLightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantLightBufDesc.MiscFlags = 0;
		constantLightBufDesc.StructureByteStride = 0;
		constantLightBufDesc.ByteWidth = sizeof(LightConstantData);

		device->CreateBuffer(&constantLightBufDesc, nullptr, &constantLightBuffer);

		D3D11_BUFFER_DESC constantPointLightBufDesc = {};
		constantPointLightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantPointLightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantPointLightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantPointLightBufDesc.MiscFlags = 0;
		constantPointLightBufDesc.StructureByteStride = 0;
		constantPointLightBufDesc.ByteWidth = sizeof(PointLightConstantData);

		device->CreateBuffer(&constantPointLightBufDesc, nullptr, &constantPointLightBuffer);

		D3D11_BLEND_DESC BlendStateLightDesc;
		ZeroMemory(&BlendStateLightDesc, sizeof(D3D11_BLEND_DESC));
		BlendStateLightDesc.RenderTarget[0].BlendEnable = TRUE;
		BlendStateLightDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		BlendStateLightDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		BlendStateLightDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		BlendStateLightDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		BlendStateLightDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		BlendStateLightDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BlendStateLightDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendStateLightDesc.IndependentBlendEnable = false;
		device->CreateBlendState(&BlendStateLightDesc, &blendStateLight);

		D3D11_DEPTH_STENCIL_DESC dsDescLess = {};
		dsDescLess.DepthEnable = TRUE;
		dsDescLess.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDescLess.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		device->CreateDepthStencilState(&dsDescLess, &depthStateLightingLess);

		D3D11_DEPTH_STENCIL_DESC dsDescGreater = {};
		dsDescGreater.DepthEnable = TRUE;
		dsDescGreater.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDescGreater.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		device->CreateDepthStencilState(&dsDescGreater, &depthStateLightingGreater);
	}

	GAMEFRAMEWORK_API void LightAPIComponent::DirectionalLightLightingPass(DirectionalLight* dirLight)
	{
		if (dirLight != nullptr && dirLight->enabled)
		{
			auto* context = DirectX11RenderAPI::Instance->context;
			auto* rtv = DirectX11RenderAPI::Instance->rtv;
			auto pDSV = DirectX11RenderAPI::Instance->pDSV;
			auto* camera = DirectX11RenderAPI::Instance->camera;
			auto* rastStateCullBack = DirectX11RenderAPI::Instance->rastStateCullBack;
			auto* viewport = DirectX11RenderAPI::Instance->viewport;
			auto& gBuffer = DirectX11RenderAPI::Instance->gBuffer;

			context->ClearState();

			context->OMSetBlendState(blendStateLight, nullptr, 0xffffffff);
			context->RSSetState(rastStateCullBack);

			context->OMSetDepthStencilState(depthStateLightingLess.Get(), 0);
			context->OMSetRenderTargets(1, &rtv, pDSV.Get());

			ID3D11ShaderResourceView* SRVs[] = { gBuffer.worldPos_DepthSRV, gBuffer.normalSRV, gBuffer.albedoSRV, gBuffer.diffuseCoeffSRV, gBuffer.specularCoeffSRV, gBuffer.ambientCoeffSRV, dirLight->shadowResourceView };
			context->PSSetShaderResources(0, 7, SRVs);

			context->RSSetViewports(1, viewport);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			LightConstantData lightData;
			lightData.cameraPos = Vector4(camera->m_position.x, camera->m_position.y, camera->m_position.z, 1.0f);
			lightData.lightParam1 = dirLight->direction;
			lightData.lightColor = dirLight->lightColor;
			lightData.DSAIntensity = Vector4(dirLight->diffuseIntensity, dirLight->specularIntensity, dirLight->ambientIntensity, 0.0f);
			lightData.CameraView = camera->GetViewMatrix();

			context->Map(constantLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, &lightData, sizeof(lightData));
			context->Unmap(constantLightBuffer, 0);

			context->PSSetConstantBuffers(0, 1, &constantLightBuffer);
			context->PSSetConstantBuffers(1, 1, &dirLight->constantLightViewProjectionBuffer);

			context->PSSetSamplers(0, 1, dirLight->comparisonSampler.GetAddressOf());

			context->VSSetShader(lightingVertexShaders[VertexLightingShaderFlags::SCREEN_QUAD], nullptr, 0);
			context->PSSetShader(lightingPixelShaders[PixelLightingShaderFlags::DIRECTIONAL], nullptr, 0);
			context->IASetInputLayout(nullptr);
			context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

			context->Draw(4, 0);
		}
	}

	GAMEFRAMEWORK_API void LightAPIComponent::PointLightLightingPass(const std::vector<PointLight*>& pointLights)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		for (PointLight* pointLight : pointLights)
		{
			if (!pointLight->enabled)
				continue;

			auto* context = DirectX11RenderAPI::Instance->context;
			auto* rtv = DirectX11RenderAPI::Instance->rtv;
			auto* camera = DirectX11RenderAPI::Instance->camera;
			auto* rastStateCullFront = DirectX11RenderAPI::Instance->rastStateCullFront;
			auto* debugRender = DirectX11RenderAPI::Instance->debugRender;
			auto& viewport = DirectX11RenderAPI::Instance->viewport;
			auto& gBuffer = DirectX11RenderAPI::Instance->gBuffer;

			context->ClearState();

			context->OMSetBlendState(blendStateLight, nullptr, 0xffffffff);

			context->RSSetState(rastStateCullFront);

			ID3D11ShaderResourceView* SRVsPoint[] = { gBuffer.worldPos_DepthSRV, gBuffer.normalSRV, gBuffer.albedoSRV, gBuffer.diffuseCoeffSRV, gBuffer.specularCoeffSRV };
			context->PSSetShaderResources(0, 5, SRVsPoint);

			context->OMSetRenderTargets(1, &rtv, nullptr);

			context->OMSetDepthStencilState(depthStateLightingGreater.Get(), 0);

			context->RSSetViewports(1, viewport);

			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

			auto lightPos = pointLight->m_gameObject->GetTransform()->GetPosition();

			LightConstantData lightData;
			lightData.cameraPos = Vector4(camera->m_position.x, camera->m_position.y, camera->m_position.z, 1.0f);
			lightData.lightParam1 = Vector4(lightPos.x, lightPos.y, lightPos.z, pointLight->range);
			lightData.lightColor = pointLight->lightColor;
			lightData.DSAIntensity = Vector4(pointLight->diffuseIntensity, pointLight->specularIntensity, 0.0f, 0.0f);
			lightData.CameraView = camera->GetViewMatrix();

			context->Map(constantLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, &lightData, sizeof(lightData));
			context->Unmap(constantLightBuffer, 0);

			ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));


			pointLight->pointLightMvp.worldMatrix = pointLight->m_gameObject->GetTransform()->GetWorldMatrix();
			pointLight->pointLightMvp.viewMatrix = camera->GetViewMatrix();
			pointLight->pointLightMvp.projectionMatrix = camera->GetProjectionMatrix();


			context->Map(constantPointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, &pointLight->pointLightMvp, sizeof(pointLight->pointLightMvp));
			context->Unmap(constantPointLightBuffer, 0);

			context->PSSetConstantBuffers(0, 1, &constantLightBuffer);
			context->VSSetConstantBuffers(2, 1, &constantPointLightBuffer);

			context->IASetInputLayout(lightingLayout);
			context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			UINT strides[] = { sizeof(Vertex) };
			UINT offsets[] = { 0 };

			context->IASetIndexBuffer(pointLight->lightMesh->m_meshPtrs[0].get()->ib, DXGI_FORMAT_R32_UINT, 0);
			context->IASetVertexBuffers(0, 1, &pointLight->lightMesh->m_meshPtrs[0].get()->vb, strides, offsets);

			context->VSSetShader(lightingVertexShaders[VertexLightingShaderFlags::NONE], nullptr, 0);
			context->PSSetShader(lightingPixelShaders[PixelLightingShaderFlags::POINT], nullptr, 0);

			int indicesLen = pointLight->lightMesh->m_meshPtrs[0].get()->indicesLen;
			context->DrawIndexed(indicesLen, 0, 0);

			debugRender->DrawSphere(2.0f, pointLight->lightColor, Matrix::CreateTranslation({ lightPos.x, lightPos.y, lightPos.z }), 16);
		}
	}

	GAMEFRAMEWORK_API void LightAPIComponent::Release()
	{
		lightingLayout->Release();
		constantLightBuffer->Release();
		constantPointLightBuffer->Release();
	}
}