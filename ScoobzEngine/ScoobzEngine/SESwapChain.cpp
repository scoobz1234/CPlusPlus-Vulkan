#include "SESwapChain.h"
#include <algorithm>

namespace ScoobzEngine {

	SESwapChain::SESwapChain(){}
	SESwapChain::~SESwapChain(){}

	void SESwapChain::Create(const VkDevice* logicalDevice, VkPhysicalDevice* physicalDevice, 
		const VkSurfaceKHR* surface,Window* windowObj, SwapChainSupportDetails swapChainSupport){
		// this is a queue of images to display...
		device = logicalDevice;
		// use the helper functions to get optimal settings...
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, *windowObj);

		// fill in data for the create info struct..
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = *surface;

		// get the proper image count
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount){
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // 2D image
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform; //orientation
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //transparency in the image
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // this clips things being blocked by other things so we dont render it
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		//attempt to create swapchain...
		if (vkCreateSwapchainKHR(*logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS){
			throw std::runtime_error("Failed to create a swap chain");
		}
		else{
			std::cout << "Swap chain creation: SUCCESSFUL!" << std::endl;
		}
		// populate swap chain image vector...
		vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(*logicalDevice, swapChain, &imageCount, swapChainImages.data());

		// stores data for chosen surface format and extent...
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

		CreateImageViews();
	}

	void SESwapChain::Cleanup(){
		for (size_t i = 0; i < swapChainImageViews.size(); i++){
			vkDestroyImageView(*device, swapChainImageViews[i], VK_NULL_HANDLE);
		}
		vkDestroySwapchainKHR(*device, swapChain, VK_NULL_HANDLE);
	}

	void SESwapChain::CleanupFramebuffers(){
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++){
			vkDestroyFramebuffer(*device, swapChainFramebuffers[i], VK_NULL_HANDLE);
		}
	}

	void SESwapChain::CreateImageViews(){
		swapChainImageViews.resize(swapChainImages.size());

		for (uint32_t i = 0; i < swapChainImages.size(); i++){
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // use 2D if using 2d texture
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; //swizzle orders color components
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; //swizzle orders color components
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; //swizzle orders color components
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; //swizzle orders color components
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(*device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS){
				throw std::runtime_error("Failed to create image views");
			}
		}
		std::cout << "Image views creation: SUCCESSFUL!" << std::endl;
	}

	void SESwapChain::CreateFramebuffers(VkRenderPass* renderPass){
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++){
			VkImageView attachments[] = { swapChainImageViews[i] };

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = *renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS){
				throw std::runtime_error("Failed to create framebuffer");
			}
		}
		std::cout << "Framebuffers creation: SUCCESSFUL!" << std::endl;
	}

	VkSurfaceFormatKHR SESwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
		// if surface has no preferred format...
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED){
			return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& currentFormat : availableFormats){
			if (currentFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
				currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
				return currentFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR SESwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes){
		for (const auto& currentMode : availablePresentModes){
			if (currentMode == VK_PRESENT_MODE_MAILBOX_KHR){
				return currentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SESwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window windowObj){
		//resolution setting
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
			return capabilities.currentExtent;
		}
		else{
			VkExtent2D actualExtent = { windowObj.windowWidth, windowObj.windowHeight };
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}
	}
}