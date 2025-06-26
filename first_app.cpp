#include "first_app.hpp"
#include "pipeline.hpp"

//std
#include <stdexcept>

namespace bm {

	FirstApp::FirstApp() {
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
		}
	}

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

	void FirstApp::createCommandBuffers() {};
	void FirstApp::drawFrame() {};
}