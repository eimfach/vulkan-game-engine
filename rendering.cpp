#include "rendering.hpp"

//std
#include <stdexcept>
#include <array>

namespace bm {

	Rendering::Rendering() {
		createPipelineLayout();
		//createPipeline();
		recreateSwapChain();
		createCommandBuffers();
	}

	Rendering::~Rendering() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void Rendering::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_info.setLayoutCount = 0;
		pipeline_layout_info.pSetLayouts = nullptr;
		pipeline_layout_info.pushConstantRangeCount = 0;
		pipeline_layout_info.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void Rendering::createPipeline() {
		assert(swapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfig pipeline_config{};
		Pipeline::defaultCfg(pipeline_config);
		pipeline_config.renderPass = swapChain->getRenderPass();
		pipeline_config.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(device, pipeline_config, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");
	}

	void Rendering::recreateSwapChain() {
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

	void Rendering::createCommandBuffers() {
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

	void Rendering::freeCommandBuffers() {
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<float>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void Rendering::recordCommandBuffer(int image_index) {
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
		clear_values[0].color = { .1f, .1f, .1f, 1.0f };
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

		pipeline->bind(commandBuffers[image_index]);
		vertexModel->bind(commandBuffers[image_index]);
		vertexModel->draw(commandBuffers[image_index]);

		vkCmdEndRenderPass(commandBuffers[image_index]);

		if (vkEndCommandBuffer(commandBuffers[image_index]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void Rendering::drawFrame() {
		uint32_t image_index{};
		auto result = swapChain->acquireNextImage(&image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire next image!");
		}

		recordCommandBuffer(image_index);
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

	void Rendering::updateVertexModel(std::vector<VertexBase>& verticies) {
		vertexModel = std::make_unique<VertexModel>(device, verticies);
	}

	void Rendering::deviceWaitIdle() {
		vkDeviceWaitIdle(device.device());
	}
	bool Rendering::windowShouldClose() {
		return window.shouldClose();
	}
}