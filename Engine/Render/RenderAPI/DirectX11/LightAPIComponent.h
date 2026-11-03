#pragma once
#include "ShaderFlags.h"
#include "Render/RenderAPI/RenderAPIComponent.h"
#include <map>
#include <vector>
#include <wrl.h>

struct ID3D11Buffer;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11BlendState;
struct ID3D11DepthStencilState;
enum class VertexLightingShaderFlags;
enum class PixelLightingShaderFlags;
class DirectionalLight;
class PointLight;

namespace RenderAPI::DirectX11
{
	class LightAPIComponent : public RenderAPIComponent
	{
	public:
		GAMEFRAMEWORK_API LightAPIComponent();
		GAMEFRAMEWORK_API void DirectionalLightLightingPass(DirectionalLight* dirLight);
		GAMEFRAMEWORK_API void PointLightLightingPass(const std::vector<PointLight*>& pointLights);

		// Inherited via RenderAPIComponent
		GAMEFRAMEWORK_API void Release() override;

	private:
		ID3D11InputLayout* lightingLayout;

		std::map<VertexLightingShaderFlags, ID3D11VertexShader*> lightingVertexShaders;
		std::map<PixelLightingShaderFlags, ID3D11PixelShader*> lightingPixelShaders;

		std::vector<VertexLightingShaderFlags> vsLightingFlags = {
			VertexLightingShaderFlags::NONE,
			VertexLightingShaderFlags::SCREEN_QUAD
		};

		std::vector<PixelLightingShaderFlags> psLightingFlags = {
			PixelLightingShaderFlags::DIRECTIONAL,
			PixelLightingShaderFlags::POINT
		};

		ID3D11Buffer* constantLightBuffer;
		ID3D11Buffer* constantPointLightBuffer;

		ID3D11BlendState* blendStateLight;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStateLightingLess;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStateLightingGreater;
	};

}
