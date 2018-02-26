#pragma once
#include "SEBuffer.h"

namespace ScoobzEngine{

	class SEVertexBuffer : public SEBuffer{

	public:
		SEVertexBuffer(const VkDevice*, const VkPhysicalDevice*, const VkSurfaceKHR*,const VkCommandPool*, const VkQueue*);
		~SEVertexBuffer();

		//Getters
		VkBuffer* GetBuffer() { return &vertexBuffer; }
		uint32_t GetVerticesSize() { return vertices.size(); }

		VkBuffer* GetIndexBuffer() { return &indexBuffer; }
		uint32_t GetIndicesSize() { return indices.size(); }

		void Create(const VkDevice*, const VkPhysicalDevice*, const VkSurfaceKHR*, const VkCommandPool*, const VkQueue*);
		void Cleanup(const VkDevice*);
		void CreateIndexBuffer(const VkDevice*, const VkPhysicalDevice*, const VkSurfaceKHR*, const VkCommandPool*, const VkQueue*);
		void CleanupIndexBuffer(const VkDevice*);

	private:

		const std::vector<Vertex> vertices = {
			{ { -0.5f, -0.5f },{ 1.0f, 1.0f, 1.0f } },
			{ {  0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
			{ {  0.5f,  0.5f },{ 0.0f, 1.0f, 0.0f } },
			{ { -0.5f,  0.5f },{ 0.0f, 0.0f, 1.0f } }
		};

		const std::vector<uint32_t> indices = {
			0,1,2,2,3,0
		};
		
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
	};
}
