#include "vertex_model.hpp"

// std
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace Biosim::Engine {
	VertexModel::VertexModel(Device& device, const VertexModel::Builder& builder) : device{device} {
		createVertexBuffers(builder.verticies);
		createIndexBuffers(builder.indicies);
	}
	VertexModel::~VertexModel() {
		vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
		vkFreeMemory(device.device(), vertexMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(device.device(), indexBuffer, nullptr);
			vkFreeMemory(device.device(), indexMemory, nullptr);
		}
	}

	void VertexModel::createVertexBuffers(const std::vector<VertexBase>& verticies) {
		vertexCount = static_cast<uint32_t>(verticies.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		VkDeviceSize buffer_size = sizeof(verticies[0]) * vertexCount;
		VkBuffer staging_buffer;
		VkDeviceMemory staging_memory;

		device.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_memory
		);

		void* data;
		if (vkMapMemory(device.device(), staging_memory, 0, buffer_size, 0, &data) != VK_SUCCESS) {
			throw std::runtime_error("Failed to map staging buffer and memory !");
		}
		memcpy(data, verticies.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(device.device(), staging_memory);

		device.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexMemory
		);

		device.copyBuffer(staging_buffer, vertexBuffer, buffer_size);
		vkDestroyBuffer(device.device(), staging_buffer, nullptr);
		vkFreeMemory(device.device(), staging_memory, nullptr);
	}

	void VertexModel::createIndexBuffers(const std::vector<uint32_t>& indicies) {
		indexCount = static_cast<uint32_t>(indicies.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize buffer_size = sizeof(indicies[0]) * indexCount;
		VkBuffer staging_buffer;
		VkDeviceMemory staging_memory;

		device.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging_buffer,
			staging_memory
		);

		void* data;
		if (vkMapMemory(device.device(), staging_memory, 0, buffer_size, 0, &data) != VK_SUCCESS) {
			throw std::runtime_error("Failed to map staging buffer and memory !");
		}
		memcpy(data, indicies.data(), static_cast<size_t>(buffer_size));
		vkUnmapMemory(device.device(), staging_memory);

		device.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexMemory
		);

		device.copyBuffer(staging_buffer, indexBuffer, buffer_size);
		vkDestroyBuffer(device.device(), staging_buffer, nullptr);
		vkFreeMemory(device.device(), staging_memory, nullptr);
	}

	void VertexModel::draw(VkCommandBuffer cmd_buffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(cmd_buffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(cmd_buffer, vertexCount, 1, 0, 0);
		}
	}

	void VertexModel::bind(VkCommandBuffer cmd_buffer) {
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd_buffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			// for small models you can safe memory by using a smaller index type
			vkCmdBindIndexBuffer(cmd_buffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
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
		attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(Vertex, position);

		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(Vertex, color);

		// extend to add new input variables for shader
		// attribute_descriptions[1].binding = 0;
		// attribute_descriptions[1].location = 2;
		// attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		// attribute_descriptions[1].offset = offsetof(Vertex, rotation);
		// 
		//return {
		//	{0,0,VK_FORMAT_R32G32_SFLOAT,offsetof(Vertex, position)},
		//	{0,1,VK_FORMAT_R32G32B32_SFLOAT,offsetof(Vertex, color)}
		//}
		return attribute_descriptions;
	}
}