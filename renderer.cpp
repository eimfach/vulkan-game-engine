#include "renderer.hpp"

//std
#include <stdexcept>
#include <array>
#include <iostream>

namespace Biosim::Engine {

	Renderer::Renderer(Window& window, Device& device) : window{ window }, device{ device } {
		std::cout << "Max size of push constants: " << device.properties.limits.maxPushConstantsSize << " Bytes \n";
		recreateSwapChain();
		createCommandBuffers();
	}

	Renderer::~Renderer() {
		freeCommandBuffers();
	}

	void Renderer::recreateSwapChain() {
		auto extent = window.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = window.getExtent();
			glfwWaitEvents();
		}

		auto result = vkDeviceWaitIdle(device.device());
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Renderer::recreateSwapChain: Device error !");
		}

		if (swapChain == nullptr) {
			swapChain = std::make_unique<SwapChain>(device, extent);
		} else {
			std::shared_ptr<SwapChain> old_swap_chain = std::move(swapChain);
			swapChain = std::make_unique<SwapChain>(device, extent, old_swap_chain);

			if (!old_swap_chain->compareSwapFormats(*swapChain.get())) {
				throw std::runtime_error("Swap chain image (or depth) format has changed!");
			}
		}
	}

	void Renderer::createCommandBuffers() {
		commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

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

	VkCommandBuffer Renderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");
		auto result = swapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to aquire swap chain image!");
		}

		isFrameStarted = true;
		auto cmd_buffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo cmd_begin{};
		cmd_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(cmd_buffer, &cmd_begin) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		return cmd_buffer;
	}


	void Renderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto cmd_buffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = swapChain->submitCommandBuffers(&cmd_buffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
			window.resetWindowResizedFlag();
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to display next image !");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
	}
	void Renderer::beginSwapChainRenderPass(VkCommandBuffer cmd_buffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass while frame is not in progress");
		assert(cmd_buffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo render_pass{};
		render_pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass.renderPass = swapChain->getRenderPass();
		render_pass.framebuffer = swapChain->getFrameBuffer(currentImageIndex);
		render_pass.renderArea.offset = { 0,0 };
		render_pass.renderArea.extent = swapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = { 0.0f, 0.01f, 0.0f, 1.0f };
		clear_values[1].depthStencil = { 1.0f, 0 };
		render_pass.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(cmd_buffer, &render_pass, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapChain->getSwapChainExtent() };
		vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
		vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
	}

	void Renderer::endSwapChainRenderPass(VkCommandBuffer cmd_buffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass while frame is not in progress");
		assert(cmd_buffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
	
		vkCmdEndRenderPass(cmd_buffer);
	}

	std::shared_ptr<Biosim::Engine::VertexModel> Renderer::createVertexModel(const std::vector<VertexBase>& verticies) {
		auto vertex_model = std::make_shared<VertexModel>(device, verticies);
		return vertex_model;
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::shared_ptr<VertexModel> Renderer::createCubeModel(glm::vec3 offset) {
		std::vector<Engine::VertexBase> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<VertexModel>(device, vertices);
	}

	void Renderer::deviceWaitIdle() {
		auto result = vkDeviceWaitIdle(device.device());
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Renderer::deviceWaitIdle: Device error !");
		}
	}
	bool Renderer::windowShouldClose() {
		return window.shouldClose();
	}
}