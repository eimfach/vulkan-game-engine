#pragma once

#include "device.hpp"
#include "buffer.hpp"
#include "vertex_base.hpp"

// libs
// don't use degrees, force use radians
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <memory>

namespace nEngine::Engine {
	class VertexModel {
	public:

		struct Vertex : VertexBase {
			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		struct Builder {
			std::vector<VertexBase> verticies{};
			std::vector<uint32_t> indicies{};

			void loadModel(const std::string& filepath);
		};

		VertexModel(Device& device, const VertexModel::Builder& builder);
		~VertexModel();

		// delete copy constructor and copy operator
		VertexModel(const VertexModel&) = delete;
		VertexModel& operator= (const VertexModel&) = delete;

		static std::pair<std::shared_ptr<VertexModel>, VertexModel::Builder>& createModelFromFile(Device& device, const std::string& filepath);

		void bind(VkCommandBuffer cmd_buffer);
		void draw(VkCommandBuffer cmd_buffer);

	private:
		Device& device;

		std::unique_ptr<Buffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer{ false };
		std::unique_ptr<Buffer> indexBuffer;
		uint32_t indexCount;

		void createVertexBuffers(const std::vector<VertexBase>& verticies);
		void createIndexBuffers(const std::vector<uint32_t>& indicies);
	};
}
