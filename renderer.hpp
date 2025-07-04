#pragma once

#include "window.hpp"
#include "swap_chain.hpp"
#include "vertex_model.hpp"
#include "vertex_base.hpp"
#include "game_object.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>
#include <iostream>

namespace Biosim::Engine {
	class Renderer {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		Renderer(Window& window, Device& device);
		~Renderer();

		// delete copy constructor and copy operator
		Renderer(const Renderer&) = delete;
		Renderer& operator= (const Renderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); }
		float getAspectRatio() const { return swapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameInProgress() && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameInProgress() && "Cannot get frame index when frame is not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer cmd_buffer);
		void endSwapChainRenderPass(VkCommandBuffer cmd_buffer);

		void deviceWaitIdle();
		bool windowShouldClose();
		std::shared_ptr<Biosim::Engine::VertexModel> createVertexModel(const std::vector<VertexBase>& verticies);
		std::shared_ptr<VertexModel> Renderer::createCubeModel(glm::vec3 offset);
	private:
		Window& window;
		Device& device;

		std::unique_ptr<SwapChain> swapChain;

		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex{};
		int currentFrameIndex{};
		bool isFrameStarted{ false };

		//void createPipelineLayout();
		//void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		//void renderObjects(const std::vector<GameObject>& objects, VkCommandBuffer cmd_buffer);
	};
}