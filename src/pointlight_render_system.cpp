#include "pointlight_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

namespace SJFGame::Engine {

	PointLightRenderSystem::PointLightRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout) : device{ device } {
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

	PointLightRenderSystem::~PointLightRenderSystem() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void PointLightRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
		//VkPushConstantRange push_constant_range{};
		//push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		//push_constant_range.offset = 0;
		//push_constant_range.size = sizeof(SimplePushConstantData);

		// the pipeline can have multiple descriptor set layouts
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{ global_set_layout };

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void PointLightRenderSystem::createPipeline(VkRenderPass render_pass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		Engine::PipelineConfig pipeline_config{};
		Engine::Pipeline::defaultCfg(pipeline_config);
		pipeline_config.attributeDescriptions.clear();
		pipeline_config.attributeDescriptions.clear();
		pipeline_config.renderPass = render_pass;
		pipeline_config.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Engine::Pipeline>(device, pipeline_config, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv");
	}

	void PointLightRenderSystem::render(Frame& frame) {

		pipeline->bind(frame.cmdBuffer);

		vkCmdBindDescriptorSets(frame.cmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frame.globalDescriptorSet,
			0, nullptr
		);

		vkCmdDraw(frame.cmdBuffer, 6, 1, 0, 0);
	}
}