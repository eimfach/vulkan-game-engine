#include "vertex_model.hpp"

// std
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace bm {
	VertexModel::VertexModel(Device& device, const std::vector<Vertex>& verticies) : device{device} {
		createVertexBuffers(verticies);
	}
	VertexModel::~VertexModel() {
		vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
		vkFreeMemory(device.device(), vertexMemory, nullptr);
	}

	void VertexModel::createVertexBuffers(const std::vector<Vertex>& verticies) {
		vertexCount = static_cast<uint32_t>(verticies.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		VkDeviceSize buffer_size = sizeof(verticies[0]) * vertexCount;
		device.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer,
			vertexMemory
		);

		void* data;
		if (vkMapMemory(device.device(), vertexMemory, 0, buffer_size, 0, &data) != VK_SUCCESS) {
			throw std::runtime_error("Failed to map vertex buffer and memory !");
		}
		memcpy(data, verticies.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(device.device(), vertexMemory);
	}

	void VertexModel::draw(VkCommandBuffer cmd_buffer) {
		vkCmdDraw(cmd_buffer, vertexCount, 1, 0, 0);
	}

	void VertexModel::bind(VkCommandBuffer cmd_buffer) {
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd_buffer, 0, 1, buffers, offsets);
	}

	std::vector<VkVertexInputBindingDescription> VertexModel::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
		binding_descriptions[0].binding = 0;
		binding_descriptions[0].stride = sizeof(Vertex);
		binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding_descriptions;
	}

	std::vector<VkVertexInputAttributeDescription> VertexModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(Vertex, position);

		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(Vertex, color);

		//return {
		//	{0,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, position)},
		//	{0,1,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, color)}
		//}
		return attribute_descriptions;
	}
}