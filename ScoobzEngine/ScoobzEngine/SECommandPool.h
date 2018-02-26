#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

namespace ScoobzEngine{

	class SECommandPool{

	public:
		SECommandPool(const VkDevice*, uint32_t, VkCommandPoolCreateFlags = 0);
		~SECommandPool();

		void Cleanup(const VkDevice*);

		VkCommandPool* GetCommandPool() { return &commandPool; }

	private:
		void createCommandPool(const VkDevice*, uint32_t, VkCommandPoolCreateFlags = 0);
		VkCommandPool commandPool;
	};
}