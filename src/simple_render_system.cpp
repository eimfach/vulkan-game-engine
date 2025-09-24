#include "simple_render_system.hpp"

#include "settings.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>
#include <mutex>

namespace nEngine::Engine {

	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout) : device{device} {
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(SimplePushConstantData);

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

	void SimpleRenderSystem::createPipeline(VkRenderPass render_pass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		Engine::PipelineConfig pipeline_config{};
		Engine::Pipeline::defaultCfg(pipeline_config);
		pipeline_config.renderPass = render_pass;
		pipeline_config.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Engine::Pipeline>(device, pipeline_config, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");
	}

	void SimpleRenderSystem::render(Frame& frame) {
		pipeline->bind(frame.cmdBuffer);

		vkCmdBindDescriptorSets(frame.cmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frame.globalDescriptorSet,
			0, nullptr
		);

		std::shared_ptr<VertexModel> previous_model = nullptr;
		auto& group = frame.ecsManager.getEntityGroup(ECS::Groups::simple_render);

		for (ECS::EntityId id : group) {
			auto& aabb = frame.ecsManager.getEntityComponent<ECS::AABB>(id);
			if (!frame.camera.isWorldSpaceAABBfrustumVisible(aabb)) continue;

			auto& transform = frame.ecsManager.getEntityComponent<ECS::Transform>(id);
			auto& mesh = frame.ecsManager.getEntityComponent<ECS::Mesh>(id);

			SimplePushConstantData push{};
			auto modelMatrix = transform.modelMatrix();
			push.modelMatrix = modelMatrix;
			push.normalMatrix = transform.normalMatrix(modelMatrix);

			std::shared_ptr<VertexModel>& model = mesh.model;
			vkCmdPushConstants(frame.cmdBuffer,
				pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			
			if (model != previous_model) {
				model->bind(frame.cmdBuffer);
				previous_model = model;
			}
			
			model->draw(frame.cmdBuffer);
		}
	}
}