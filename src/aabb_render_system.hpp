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
	class AABBRenderSystem {
	public:
		AABBRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
		~AABBRenderSystem();

		// delete copy constructor and copy operator
		AABBRenderSystem(const AABBRenderSystem&) = delete;
		AABBRenderSystem& operator= (const AABBRenderSystem&) = delete;

		void render(Frame& frame, std::vector<ECS::AABB>& boxes);

	private:
		Device& device;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Engine::Pipeline> pipeline;

		void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
		void createPipeline(VkRenderPass render_pass);
	};
}
