#include "VulkanRenderAPI.h"
#include "DisplayWin.h"
#include <memory>
#include <GLFW/glfw3.h>
#include <iostream>

namespace RenderAPI::Vulkan
{
	const std::vector<char const*> validationLayers = {
"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif

	GAMEFRAMEWORK_API VulkanRenderAPI::VulkanRenderAPI(int screenWidth, int screenHeight, DisplayWin* displayWin)
	{
		try
		{
			CreateInstance();
			SetupDebugMessenger();
		}
		catch (const vk::SystemError& err)
		{
			std::cerr << "Vulkan error: " << err.what() << std::endl;
			return;
		}
	}

	GAMEFRAMEWORK_API void VulkanRenderAPI::CreateInstance()
	{
		constexpr vk::ApplicationInfo appInfo{ .pApplicationName = "First triangle",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = vk::ApiVersion14 };

		auto requiredLayers = GetRequiredLayers();
		CheckLayersAvailability(requiredLayers);

		auto requiredExtensions = GetRequiredExtensions();
		CheckExtensionsAvailability(requiredExtensions);

		vk::InstanceCreateInfo createInfo{
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
			.ppEnabledLayerNames = requiredLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
			.ppEnabledExtensionNames = requiredExtensions.data(),
		};

		instance = vk::raii::Instance(context, createInfo);
	}

	GAMEFRAMEWORK_API void VulkanRenderAPI::SetupDebugMessenger()
	{
		if (!enableValidationLayers) 
			return;

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning 
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
		);

		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
			| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		);

		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{ .messageSeverity = severityFlags,
																		  .messageType = messageTypeFlags,
																		  .pfnUserCallback = &DebugCallback };

		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);

	}

	GAMEFRAMEWORK_API void VulkanRenderAPI::Render(float deltaTime)
	{
	}

	GAMEFRAMEWORK_API void VulkanRenderAPI::Release()
	{
	}

	GAMEFRAMEWORK_API std::vector<const char*> VulkanRenderAPI::GetRequiredExtensions()
	{
		// Get the required instance extensions from GLFW.
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		return extensions;
	}

	GAMEFRAMEWORK_API void VulkanRenderAPI::CheckExtensionsAvailability(std::vector<const char*> requiredExtensions)
	{
		auto extensionProperties = context.enumerateInstanceExtensionProperties();

		std::cout << "available extensions:\n";

		for (const auto& extension : extensionProperties) {
			std::cout << '\t' << extension.extensionName << '\n';
		}

		std::cout << "\nrequired extensions:\n";

		for (const auto& extension : requiredExtensions) {
			std::cout << '\t' << extension << '\n';
		}

		// Check if the required extensions are supported by the Vulkan implementation.
		for (uint32_t i = 0; i < requiredExtensions.size(); ++i)
		{
			if (std::ranges::none_of(extensionProperties,
				[extension = requiredExtensions[i]](auto const& extensionProperty)
				{ return strcmp(extensionProperty.extensionName, extension) == 0; }))
			{
				throw std::runtime_error("Required extension not supported: " + std::string(requiredExtensions[i]));
			}
		}

		std::cout << "\n---------------------------------\n";
	}

	GAMEFRAMEWORK_API std::vector<const char*> VulkanRenderAPI::GetRequiredLayers()
	{
		std::vector<const char*> requiredLayers;

		if (enableValidationLayers)
		{
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}

		return requiredLayers;
	}

	GAMEFRAMEWORK_API void VulkanRenderAPI::CheckLayersAvailability(std::vector<const char*> requiredLayers)
	{
		auto layerProperties = context.enumerateInstanceLayerProperties();

		std::cout << "\navailable layers:\n";

		for (const auto& layer : layerProperties) {
			std::cout << '\t' << layer.layerName << '\n';
		}

		std::cout << "\nrequired layers:\n";

		for (const auto& layer : requiredLayers) {
			std::cout << '\t' << layer << '\n';
		}

		// Check if the required layers are supported by the Vulkan implementation.
		for (uint32_t i = 0; i < requiredLayers.size(); ++i)
		{
			if (std::ranges::none_of(layerProperties,
				[layer = requiredLayers[i]](auto const& layerProperty)
				{ return strcmp(layerProperty.layerName, layer) == 0; }))
			{
				throw std::runtime_error("Required layer not supported: " + std::string(requiredLayers[i]));
			}
		}

		std::cout << "\n---------------------------------\n";
	}

	GAMEFRAMEWORK_API VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanRenderAPI::DebugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT	  severity, 
		vk::DebugUtilsMessageTypeFlagsEXT			  type, 
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		/*if (severity <= vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
			return;*/

		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

		return vk::False;
	}
}