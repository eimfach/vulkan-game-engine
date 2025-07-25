#include "line_render_system.hpp"

#include "settings.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

namespace SJFGame::Engine {

	struct LinePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::vec3 color{1.f, 1.f, 1.f};
	};

	LineRenderSystem::LineRenderSystem(Device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout) : device{ device } {
		createPipelineLayout(global_set_layout);
		createPipeline(render_pass);
	}

	LineRenderSystem::~LineRenderSystem() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void LineRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(LinePushConstantData);

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

	void LineRenderSystem::createPipeline(VkRenderPass render_pass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfig pipeline_config{};
		Pipeline::defaultCfg(pipeline_config);
		Pipeline::setTopology(pipeline_config, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, VK_POLYGON_MODE_LINE);
		pipeline_config.renderPass = render_pass;
		pipeline_config.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Engine::Pipeline>(device, pipeline_config, "shaders/line.vert.spv", "shaders/line.frag.spv");
	}

	void LineRenderSystem::render(Frame& frame, std::vector<ECS::AABB> boxes) {
		pipeline->bind(frame.cmdBuffer);

		vkCmdBindDescriptorSets(frame.cmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frame.globalDescriptorSet,
			0, nullptr
		);

		for (ECS::AABB aabb: boxes) {
			LinePushConstantData push{};
			//auto& model = meshes[id].model;

			//vkCmdPushConstants(frame.cmdBuffer,
			//	pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			//	0,
			//	sizeof(LinePushConstantData),
			//	&push);

			//model->bind(frame.cmdBuffer);
			//model->draw(frame.cmdBuffer);
		}
	}

	void LineRenderSystem::render(Frame& frame) {

		pipeline->bind(frame.cmdBuffer);

		vkCmdBindDescriptorSets(frame.cmdBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frame.globalDescriptorSet,
			0, nullptr
		);

		auto& group = frame.ecsManager.getEntityGroup<ECS::Transform, ECS::Mesh, ECS::Color, ECS::Visibility, ECS::RenderLines>();
		for (ECS::EntityId id : group) {

			auto& transform = frame.ecsManager.getEntityComponent<ECS::Transform>(id);
			auto& mesh = frame.ecsManager.getEntityComponent<ECS::Mesh>(id);
			auto& color = frame.ecsManager.getEntityComponent<ECS::Color>(id);

			// cull near and far plane (just for testing)
			glm::vec3 direction_to_camera{ transform.translation - frame.camera.getPosition()};
			float distance_forward{ glm::dot(direction_to_camera, glm::vec3{0.f,0.f,1.f}) };
			if (distance_forward < Settings::NEAR_PLANE || distance_forward > Settings::FAR_PLANE) {
				continue;
			}

			LinePushConstantData push{};
			auto model_matrix = transform.mat4();
			push.modelMatrix = model_matrix;
			push.color = color.rgb; //TODO: crashes because indicies are disjoint
			auto& model = mesh.model;

			vkCmdPushConstants(frame.cmdBuffer,
				pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(LinePushConstantData),
				&push);

			model->bind(frame.cmdBuffer);
			model->draw(frame.cmdBuffer);
		}
	}
}