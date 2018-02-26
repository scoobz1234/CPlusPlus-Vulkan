#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "window.h"

namespace ScoobzEngine {

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class SESwapChain{

	public:
		SESwapChain();~SESwapChain();

		void Create(const VkDevice*, VkPhysicalDevice*, const VkSurfaceKHR*, Window*, SwapChainSupportDetails);
		void Cleanup();
		void CleanupFramebuffers();
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR&, Window);
		void CreateFramebuffers(VkRenderPass*);
		void CreateImageViews();

		//Getters
		VkSwapchainKHR* GetSwapchain() { return &swapChain; }
		VkFormat* GetImageFormat() { return &swapChainImageFormat; }
		VkExtent2D* GetExtent() { return &swapChainExtent; }
		size_t GetFramebufferSize() { return swapChainFramebuffers.size(); }
		VkFramebuffer* GetFramebuffer(unsigned int index) { return &swapChainFramebuffers[index]; }


	private:

		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;

		// store swap chain details
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		const VkDevice* device;
	};
}
