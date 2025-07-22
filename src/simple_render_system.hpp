#pragma once

#include "camera.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "pipeline.hpp"
#include "frame.hpp"
#include "entity_manager.hpp"

// std
#include <memory>
#include <vector>

namespace SJFGame::Engine {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~SimpleRenderSystem();

		// delete copy constructor and copy operator
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator= (const SimpleRenderSystem&) = delete;

		void render(Frame& frame);

	private:
		Device& device;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Engine::Pipeline> pipeline;


		void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
		void createPipeline(VkRenderPass render_pass);
	};
}