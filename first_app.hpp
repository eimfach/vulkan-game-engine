#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"

// std
#include <memory>
#include <vector>

namespace bm {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		FirstApp();
		~FirstApp();

		// delete copy constructor and copy operator
		FirstApp(const FirstApp&) = delete;
		FirstApp& operator= (const FirstApp&) = delete;

		void run();

	private:
		Window window{WIDTH, HEIGHT, "Hello Vulkan!"};
		Device device{ window };
		SwapChain swapChain{ device, window.getExtent() };
		VkPipelineLayout pipelineLayout;
		
		std::unique_ptr<Pipeline> pipeline;
		std::vector<VkCommandBuffer> commandBuffers;

		//Pipeline pipeline{ device, Pipeline::defaultCfg(WIDTH, HEIGHT), "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};

		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();
	};
}