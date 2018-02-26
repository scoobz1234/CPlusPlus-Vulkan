#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <fstream>
#include <map>
#include <set>

#include "Window.h"
#include "SEVertex.h"
#include "SESwapChain.h"
#include "SEVertexBuffer.h"
#include "SEQueueFamily.h"
#include "SECommandPool.h"

namespace ScoobzEngine{

	static std::vector<char> ReadFile(const std::string& filename){
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()){
			throw std::runtime_error("Failed to open file");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	class ScoobzEngine{

	public:
		ScoobzEngine();~ScoobzEngine();
		
		//FUNCTIONS :: PUBLIC
		void Initvulkan();
		void CleanupVulkan(); 
		void GameLoop();

		//HANDLES :: PUBLIC
		Window windowObj;

	private:
		//FUNCTIONS :: PRIVATE
		static void OnWindowResized(GLFWwindow*, int, int);
		void InitWindow();
		void CreateInstance();
		void CleanupSwapChain();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void SetupDebugCallback();
		void CreateSurface();
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
		void GetPhysicalDevices();
		int RateDeviceSuitability(VkPhysicalDevice);
		void CreateLogicalDevice();
		void RecreateSwapChain();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		VkShaderModule CreateShaderModule(const std::vector<char>&);
		void CreateCommandBuffers();
		void CreateSemaphores();
		void DrawFrame();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice);

		VkResult CreateDebugReportCallbackEXT(VkInstance, const VkDebugReportCallbackCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugReportCallbackEXT*);
		static void DestroyDebugReportCallbackEXT(VkInstance, VkDebugReportCallbackEXT, const VkAllocationCallbacks*);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*);

		const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
		const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		//HANDLES :: PRIVATE
		VkInstance instance;
		VkDebugReportCallbackEXT callback;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice;
		VkQueue graphicsQueue;
		VkQueue transferQueue;
		SESwapChain* swapchain;
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		SECommandPool* graphicsCommandPool;
		SECommandPool* transferCommandPool;
		SEVertexBuffer* vertexBuffer;
		std::vector<VkCommandBuffer> commandBuffers;
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;

		
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif //NDEBUG
	};
}
