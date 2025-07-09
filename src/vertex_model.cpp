#include "vertex_model.hpp"

#include "utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// TODO: need better solution
#ifndef ENGINE_DIR
#define ENGINE_DIR "./"
#endif

// std
#include <cassert>
#include <stdexcept>
#include <unordered_map>

namespace std {
	template<>
	struct hash<SJFGame::Engine::VertexBase> {
		size_t operator()(SJFGame::Engine::VertexBase const &vertex) const {
			size_t seed = 0;
			SJFGame::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace SJFGame::Engine {
	VertexModel::VertexModel(Device& device, const VertexModel::Builder& builder) : device{device} {
		createVertexBuffers(builder.verticies);
		createIndexBuffers(builder.indicies);
	}
	VertexModel::~VertexModel() {}

	std::shared_ptr<VertexModel> VertexModel::createModelFromFile(Device& device, const std::string& filepath) {
		Builder builder{};
		builder.loadModel(ENGINE_DIR + filepath);
		return std::make_shared<VertexModel>(device, builder);
	}

	void VertexModel::createVertexBuffers(const std::vector<VertexBase>& verticies) {
		vertexCount = static_cast<uint32_t>(verticies.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");

		VkDeviceSize buffer_size = sizeof(verticies[0]) * vertexCount;
		uint32_t vertex_size = sizeof(verticies[0]);

		Buffer staging_buffer{
			device,
			vertex_size,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		staging_buffer.map();
		staging_buffer.writeToBuffer((void *)verticies.data());

		vertexBuffer = std::make_unique<Buffer>(
			device,
			vertex_size,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		device.copyBuffer(staging_buffer.getBuffer(), vertexBuffer->getBuffer(), buffer_size);
	}

	void VertexModel::createIndexBuffers(const std::vector<uint32_t>& indicies) {
		indexCount = static_cast<uint32_t>(indicies.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize buffer_size = sizeof(indicies[0]) * indexCount;
		uint32_t index_size = sizeof(indicies[0]);

		Buffer staging_buffer{
			device,
			index_size,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		staging_buffer.map();
		staging_buffer.writeToBuffer((void*)indicies.data());

		indexBuffer = std::make_unique<Buffer>(
			device,
			index_size,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		device.copyBuffer(staging_buffer.getBuffer(), indexBuffer->getBuffer(), buffer_size);
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
		VkBuffer buffers[] = { vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(cmd_buffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			// for small models you can safe memory by using a smaller index type
			vkCmdBindIndexBuffer(cmd_buffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
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
		std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
		
		attribute_descriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attribute_descriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attribute_descriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attribute_descriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

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

	void VertexModel::Builder::loadModel(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		verticies.clear();
		indicies.clear();

		std::unordered_map<VertexBase, uint32_t> unique_verticies{};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				VertexBase vertex{};

				if (index.vertex_index >= 0) {
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};
					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};
					
				}
				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}
				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.vertex_index + 0],
						attrib.texcoords[2 * index.vertex_index + 1],
					};
				}

				if (unique_verticies.count(vertex) == 0) {
					unique_verticies[vertex] = static_cast<uint32_t>(verticies.size());
					verticies.push_back(vertex);
				}
				indicies.push_back(unique_verticies[vertex]);
			}
		}
	}
}