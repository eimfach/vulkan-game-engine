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
	class PointLightRenderSystem {
	public:
		PointLightRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~PointLightRenderSystem();

		// delete copy constructor and copy operator
		PointLightRenderSystem(const PointLightRenderSystem&) = delete;
		PointLightRenderSystem& operator= (const PointLightRenderSystem&) = delete;

		void update(Frame& frame, GlobalUniformBufferOutput& ubo);
		void render(const Frame& frame);

	private:
		Device& device;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Engine::Pipeline> pipeline;

		void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
		void createPipeline(VkRenderPass render_pass);
	};
}