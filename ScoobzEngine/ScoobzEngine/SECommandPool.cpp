#include "SECommandPool.h"

namespace ScoobzEngine{

	SECommandPool::SECommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags){
		createCommandPool(logicalDevice, queueFamilyIndex, flags);
	}

	SECommandPool::~SECommandPool(){}

	void SECommandPool::Cleanup(const VkDevice* logicalDevice){
		vkDestroyCommandPool(*logicalDevice, commandPool, VK_NULL_HANDLE);
	}

	void SECommandPool::createCommandPool(const VkDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags){
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = flags;

		if (vkCreateCommandPool(*logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS){
			throw std::runtime_error("Failed to create command pool");
		}
		else{
			std::cout << "Command Pool created successfully" << std::endl;
		}
	}
}