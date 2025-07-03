#pragma once

#include "device.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace Biosim::Engine {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(Device& device, VkRenderPass render_pass);
		~SimpleRenderSystem();

		// delete copy constructor and copy operator
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator= (const SimpleRenderSystem&) = delete;

		void renderObjects(const std::vector<GameObject>& objects, VkCommandBuffer cmd_buffer);

	private:
		Device& device;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Engine::Pipeline> pipeline;

		void createPipelineLayout();
		void createPipeline(VkRenderPass render_pass);
	};
}