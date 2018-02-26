#pragma once

#include "SEVertex.h"
#include <iostream>
#include <vector>

namespace ScoobzEngine {

	class SEBuffer{

	public:
		SEBuffer();~SEBuffer();

	protected:
		void CreateBuffer(const VkDevice*, const VkPhysicalDevice*,	const VkSurfaceKHR*, VkDeviceSize, VkBufferUsageFlags, 
			VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);

		void CleanupBuffer(const VkDevice*, VkBuffer&, VkDeviceMemory&);

		uint32_t findMemoryType(VkPhysicalDevice, uint32_t, VkMemoryPropertyFlags);

		void CopyBuffer(const VkDevice*, const VkCommandPool*, VkBuffer, VkBuffer, VkDeviceSize, const VkQueue*);
	};
}

