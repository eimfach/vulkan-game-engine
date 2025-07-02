#include "renderer.hpp"

// libs
// don't use degrees, force use radians
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>

namespace Biosim::Engine {

	struct SimplePushConstantData {
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		glm::float32 rotation{};
		alignas(16) glm::vec3 color;
	};

	Renderer::Renderer() {
		std::cout << "Max size of push constants: " << device.properties.limits.maxPushConstantsSize << " Bytes \n";
		createPipelineLayout();
		//createPipeline();
		recreateSwapChain();
		createCommandBuffers();
	}

	Renderer::~Renderer() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void Renderer::createPipelineLayout() {

		VkPushConstantRange push_constant_range{};
		push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		push_constant_range.offset = 0;
		push_constant_range.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 1;
		pipeline_layout_info.pPushConstantRanges = &push_constant_range;

		if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void Renderer::createPipeline() {
		assert(swapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfig pipeline_config{};
		Pipeline::defaultCfg(pipeline_config);
		pipeline_config.renderPass = swapChain->getRenderPass();
		pipeline_config.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(device, pipeline_config, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");
	}

	void Renderer::recreateSwapChain() {
		auto extent = window.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = window.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device.device());
		if (swapChain == nullptr) {
			swapChain = std::make_unique<SwapChain>(device, extent);
		}
		else {
			swapChain = std::make_unique<SwapChain>(device, extent, std::move(swapChain));
			freeCommandBuffers();
			createCommandBuffers();
		}


		// TODO: if render pass compatible do nothing else
		createPipeline();
	}

	void Renderer::createCommandBuffers() {
		commandBuffers.resize(swapChain->imageCount());
		VkCommandBufferAllocateInfo cmd_alloc{};
		cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_alloc.commandPool = device.getCommandPool();
		cmd_alloc.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &cmd_alloc, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void Renderer::freeCommandBuffers() {
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void Renderer::recordCommandBuffer(const std::vector<GameObject>& objects, int image_index) {
		VkCommandBufferBeginInfo cmd_begin{};
		cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffers[image_index], &cmd_begin) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo render_pass{};
		render_pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass.renderPass = swapChain->getRenderPass();
		render_pass.framebuffer = swapChain->getFrameBuffer(image_index);
		render_pass.renderArea.offset = { 0,0 };
		render_pass.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = { 0.0f, 0.01f, 0.0f, 1.0f };
		clear_values[1].depthStencil = { 1.0f, 0 };
		render_pass.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(commandBuffers[image_index], &render_pass, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[image_index], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[image_index], 0, 1, &scissor);

		renderObjects(objects, commandBuffers[image_index]);

		vkCmdEndRenderPass(commandBuffers[image_index]);

		if (vkEndCommandBuffer(commandBuffers[image_index]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void Renderer::renderObjects(const std::vector<GameObject>& objects, VkCommandBuffer cmd_buffer) {
		pipeline->bind(cmd_buffer);

		for (auto& obj : objects) {
			SimplePushConstantData push{};
			push.offset = obj.transform2D.translation;
			push.color = obj.color;
			push.transform = obj.transform2D.mat2();
			push.rotation = obj.transform2D.rotation;

			vkCmdPushConstants(cmd_buffer, 
				pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
				0, 
				sizeof(SimplePushConstantData), 
				&push);
			obj.model->bind(cmd_buffer);
			obj.model->draw(cmd_buffer);
		}
	}

	void Renderer::drawFrame(const std::vector<GameObject>& objects) {
		uint32_t image_index{};
		auto result = swapChain->acquireNextImage(&image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire next image!");
		}

		recordCommandBuffer(objects, image_index);
		result = swapChain->submitCommandBuffers(&commandBuffers[image_index], &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
			window.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to display next image!");
		}
	};

	std::shared_ptr<Biosim::Engine::VertexModel> Renderer::createVertexModel(const std::vector<VertexBase>& verticies) {
		auto vertex_model = std::make_shared<VertexModel>(device, verticies);
		return vertex_model;
	}

	void Renderer::deviceWaitIdle() {
		vkDeviceWaitIdle(device.device());
	}
	bool Renderer::windowShouldClose() {
		return window.shouldClose();
	}
}