#pragma once
#include "Render/RenderAPI/RenderAPI.h"
#include <string>
#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

class DisplayWin;

namespace RenderAPI::Vulkan
{
	class VulkanRenderAPI : public RenderAPI
	{
	public:
		GAMEFRAMEWORK_API VulkanRenderAPI(int screenWidth, int screenHeight, DisplayWin* displayWin);

		GAMEFRAMEWORK_API void CreateInstance();
		GAMEFRAMEWORK_API void SetupDebugMessenger();

		// Inherited via RenderAPI
		GAMEFRAMEWORK_API void Render(float deltaTime) override;
		GAMEFRAMEWORK_API void Release() override;

	private:
		GAMEFRAMEWORK_API std::vector<const char*> GetRequiredExtensions();
		GAMEFRAMEWORK_API void CheckExtensionsAvailability(std::vector<const char*> requiredExtensions);

		GAMEFRAMEWORK_API std::vector<const char*> GetRequiredLayers();
		GAMEFRAMEWORK_API void CheckLayersAvailability(std::vector<const char*> requiredLayers);

		GAMEFRAMEWORK_API static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
			vk::DebugUtilsMessageTypeFlagsEXT             type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		vk::raii::Context  context;
		vk::raii::Instance instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	};
}

