#pragma once

#include "camera.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"
#include "frame.hpp"

// std
#include <memory>
#include <vector>

namespace nEngine::Engine {
	class LineRenderSystem {
	public:
		LineRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~LineRenderSystem();

		// delete copy constructor and copy operator
		LineRenderSystem(const LineRenderSystem&) = delete;
		LineRenderSystem& operator= (const LineRenderSystem&) = delete;

		void render(Frame& frame);

	private:
		Device& device;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Engine::Pipeline> pipeline;

		void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
		void createPipeline(VkRenderPass render_pass);
	};
}
