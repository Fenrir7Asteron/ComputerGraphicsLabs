#include "VulkanRenderAPI.h"
#include "DisplayWin.h"
#include <memory>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <ranges>

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
			PickPhysicalDevice();
			CreateLogicalDevice();
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

	GAMEFRAMEWORK_API void VulkanRenderAPI::PickPhysicalDevice()
	{
		auto physicalDevices = instance.enumeratePhysicalDevices();

		if (physicalDevices.empty())
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Use an ordered map to automatically sort candidates by increasing score
		std::multimap<int, vk::raii::PhysicalDevice> candidates;

		for (const auto& pd : physicalDevices)
		{
			auto deviceProperties = pd.getProperties2();
			auto deviceFeatures = pd.getFeatures2();
			auto queueFamilies = pd.getQueueFamilyProperties2();
			auto availableDeviceExtensions = pd.enumerateDeviceExtensionProperties();

			std::vector<const char*> requiredDeviceExtension = { vk::KHRSwapchainExtensionName };
			int score = 0;

			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				score += 1000;
			}

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.properties.limits.maxImageDimension2D;

			// Minimal required Vulkan version is 1.3
			if (deviceProperties.properties.apiVersion <= vk::ApiVersion12)
				continue;

			bool supportsGraphics = std::ranges::any_of(queueFamilies,
				[](vk::QueueFamilyProperties2 const& family) 
			{
				return !!(family.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
			});
			if (!supportsGraphics)
				continue;

			bool supportsAllRequiredExtensions =
				std::ranges::all_of(requiredDeviceExtension, 
					[&availableDeviceExtensions](auto const& requiredExtension)
					{
						return std::ranges::any_of(
							availableDeviceExtensions,
							[&requiredExtension](vk::ExtensionProperties const& availableExtension)
							{
								return strcmp(availableExtension.extensionName, requiredExtension) == 0;
							});
					});
			if (!supportsAllRequiredExtensions)
				continue;

			// Check if the physicalDevice supports the required features
			auto features = pd.template getFeatures2<vk::PhysicalDeviceFeatures2,
				vk::PhysicalDeviceVulkan11Features,
				vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
			bool supportsRequiredFeatures = 
				deviceFeatures.features.geometryShader &&
				features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
				features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
				features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState
			;

			if (!supportsRequiredFeatures)
				continue;

			candidates.insert(std::make_pair(score, pd));
		}

		// Check if the best candidate is suitable at all
		if (!candidates.empty() && candidates.rbegin()->first > 0)
		{
			this->physicalDevice = candidates.rbegin()->second;
		}
		else
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	GAMEFRAMEWORK_API void VulkanRenderAPI::CreateLogicalDevice()
	{
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		auto graphicsQueueFamilyProperty =
			std::ranges::find_if(queueFamilyProperties, [](auto const& family)
				{ return (family.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0); });

		auto graphicsIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

		float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ 
			.queueFamilyIndex = graphicsIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		vk::PhysicalDeviceFeatures deviceFeatures;

		// Create a chain of feature structures
		vk::StructureChain<vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan11Features,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
			featureChain = {
				{},                                    
				{.shaderDrawParameters = true},        
				{.dynamicRendering = true},            
				{.extendedDynamicState = true}         
		};

		std::vector<const char*> requiredDeviceExtension = {
			vk::KHRSwapchainExtensionName 
		};

		vk::DeviceCreateInfo deviceCreateInfo{
			.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &deviceQueueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
			.ppEnabledExtensionNames = requiredDeviceExtension.data()
		};

		device = vk::raii::Device(physicalDevice, deviceCreateInfo);
		graphicsQueue = vk::raii::Queue(device, graphicsIndex, 0);
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