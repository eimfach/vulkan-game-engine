#include "aabb_render_system.hpp"

#include "settings.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>
#include <iostream>

namespace SJFGame::Engine {

	struct AABBPushConstantData {
		glm::vec3 color{ .1f, .9f, .05f };
	};

	AABBRenderSystem::AABBRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout) : device{ device } {
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

	AABBRenderSystem::~AABBRenderSystem() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void AABBRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(AABBPushConstantData);

		// the pipeline can have multiple descriptor set layouts
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts{ global_set_layout };

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;

		if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void AABBRenderSystem::createPipeline(VkRenderPass render_pass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfig pipeline_config{};
		Pipeline::defaultCfg(pipeline_config);
		//Pipeline::setTopology(pipeline_config, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL);
		Pipeline::setTopology(pipeline_config, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_LINE);
		//Pipeline::setTopology(pipeline_config, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_POLYGON_MODE_POINT);
		pipeline_config.renderPass = render_pass;
		pipeline_config.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Engine::Pipeline>(device, pipeline_config, "shaders/aabb.vert.spv", "shaders/aabb.frag.spv");
	}

	void AABBRenderSystem::render(Frame& frame, std::vector<ECS::AABB>& boxes) {
		// cull near and far plane (just for testing)
		//glm::vec3 direction_to_camera{ transform.translation - frame.camera.getPosition() };
		//float distance_forward{ glm::dot(direction_to_camera, glm::vec3{0.f,0.f,1.f}) };
		//if (distance_forward < Settings::NEAR_PLANE || distance_forward > Settings::FAR_PLANE) {
		//	continue;
		//}
		pipeline->bind(frame.cmdBuffer);

		vkCmdBindDescriptorSets(frame.cmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frame.globalDescriptorSet,
			0, nullptr
		);

		auto& group = frame.ecsManager.getEntityGroup<ECS::Identification, ECS::Transform, ECS::Mesh, ECS::Visibility, ECS::AABB>();
		for (ECS::EntityId id : group) {
			auto& aabb = frame.ecsManager.getEntityComponent<ECS::AABB>(id);
			if (!frame.camera.isWorldSpaceAABBfrustumVisible(aabb)) continue;

			auto& transform = frame.ecsManager.getEntityComponent<ECS::Transform>(id);
			auto& mesh = frame.ecsManager.getEntityComponent<ECS::Mesh>(id);

			AABBPushConstantData push{};

			auto& model = aabb.model;
			vkCmdPushConstants(frame.cmdBuffer,
				pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(AABBPushConstantData),
				&push);

			model->bind(frame.cmdBuffer);
			model->draw(frame.cmdBuffer);
		}
	}
}