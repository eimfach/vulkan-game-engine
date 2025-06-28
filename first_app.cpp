#include "first_app.hpp"
#include "pipeline.hpp"

//std
#include <stdexcept>
#include <array>

namespace bm {

	FirstApp::FirstApp() {
		loadModels();
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run() {
		while (!window.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(device.device());
	}

	void FirstApp::loadModels() {
		float y{ 0.45 };
		std::vector<VertexModel::Vertex> verticies{
			{{0.0f, -y}, {1.0f, 0.0f, 0.0f}},
			{{y, y}, {0.0f, 1.0f, 0.0f}},
			{{-y, y}, {0.0f, 0.0f, 1.0f}},
		};

		vertexModel = std::make_unique<VertexModel>(device, verticies);
	};

	void FirstApp::createPipelineLayout() {
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

	void FirstApp::createPipeline() {
		auto pipeline_config = Pipeline::defaultCfg(swapChain.width(), swapChain.height());
		pipeline_config.renderPass = swapChain.getRenderPass();
		pipeline_config.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(device, pipeline_config, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");
	}

	void FirstApp::createCommandBuffers() {
		commandBuffers.resize(swapChain.imageCount());
		VkCommandBufferAllocateInfo cmd_alloc{};
		cmd_alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_alloc.commandPool = device.getCommandPool();
		cmd_alloc.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &cmd_alloc, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo cmd_begin{};
			cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(commandBuffers[i], &cmd_begin) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo render_pass{};
			render_pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass.renderPass = swapChain.getRenderPass();
			render_pass.framebuffer = swapChain.getFrameBuffer(i);
			render_pass.renderArea.offset = { 0,0 };
			render_pass.renderArea.extent = swapChain.getSwapChainExtent();
			
			std::array<VkClearValue, 2> clear_values{};
			clear_values[0].color = { .1f, .1f, .1f, 1.0f };
			clear_values[1].depthStencil = { 1.0f, 0 };
			render_pass.clearValueCount = static_cast<uint32_t>(clear_values.size());
			render_pass.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(commandBuffers[i], &render_pass, VK_SUBPASS_CONTENTS_INLINE);
			pipeline->bind(commandBuffers[i]);
			vertexModel->bind(commandBuffers[i]);
			vertexModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	void FirstApp::drawFrame() {
		uint32_t image_index{};
		auto result = swapChain.acquireNextImage(&image_index);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire next image!");
		}

		result = swapChain.submitCommandBuffers(&commandBuffers[image_index], &image_index);

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to display next image!");
		}
	};
}