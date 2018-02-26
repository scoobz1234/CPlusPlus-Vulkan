#include "SEVertexBuffer.h"

namespace ScoobzEngine{

	SEVertexBuffer::SEVertexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
		const VkCommandPool* transferCommandPool, const VkQueue* transferQueue){
		Create(logicalDevice, physicalDevice, surface, transferCommandPool, transferQueue);
		CreateIndexBuffer(logicalDevice, physicalDevice, surface, transferCommandPool, transferQueue);
	}
	
	SEVertexBuffer::~SEVertexBuffer(){}

	void SEVertexBuffer::Create(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
		const VkCommandPool* transferCommandPool, const VkQueue* transferQueue){
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(*logicalDevice, stagingBufferMemory);

		CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		CopyBuffer(logicalDevice, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize, transferQueue);

		vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
	}

	void SEVertexBuffer::Cleanup(const VkDevice* logicalDevice){
		CleanupBuffer(logicalDevice, vertexBuffer, vertexBufferMemory);
	}

	void SEVertexBuffer::CreateIndexBuffer(const VkDevice* logicalDevice, const VkPhysicalDevice* physicalDevice, const VkSurfaceKHR* surface,
		const VkCommandPool* transferCommandPool, const VkQueue* transferQueue){
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(*logicalDevice, stagingBufferMemory);

		CreateBuffer(logicalDevice, physicalDevice, surface, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(logicalDevice, transferCommandPool, stagingBuffer, indexBuffer, bufferSize, transferQueue);

		vkDestroyBuffer(*logicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(*logicalDevice, stagingBufferMemory, nullptr);
	}

	void SEVertexBuffer::CleanupIndexBuffer(const VkDevice* logicalDevice){
		CleanupBuffer(logicalDevice, indexBuffer, indexBufferMemory);
	}


}