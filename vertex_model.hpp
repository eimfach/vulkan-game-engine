#pragma once

#include "device.hpp"

// libs
// don't use degrees, force use radians
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

// std
#include <vector>

namespace bm {
	class VertexModel {
	public:

		struct Vertex {
			glm::vec2 position;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		VertexModel(Device& device, const std::vector<Vertex>& verticies);
		~VertexModel();

		// delete copy constructor and copy operator
		VertexModel(const VertexModel&) = delete;
		VertexModel& operator= (const VertexModel&) = delete;

		void bind(VkCommandBuffer cmd_buffer);
		void draw(VkCommandBuffer cmd_buffer);

	private:
		Device& device;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		uint32_t vertexCount;

		void createVertexBuffers(const std::vector<Vertex>& verticies);
	};
}
