#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "vertex_model.hpp"
#include "vertex_base.hpp"

// std
#include <memory>
#include <vector>

namespace bm {
	class Rendering {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		Rendering();
		~Rendering();

		// delete copy constructor and copy operator
		Rendering(const Rendering&) = delete;
		Rendering& operator= (const Rendering&) = delete;

		void drawFrame();
		void deviceWaitIdle();
		bool windowShouldClose();
		void updateVertexModel(std::vector<VertexBase>& verticies);

	private:
		Window window{ WIDTH, HEIGHT, "Hello Vulkan!" };
		Device device{ window };
		VkPipelineLayout pipelineLayout;

		std::unique_ptr<SwapChain> swapChain;
		std::unique_ptr<Pipeline> pipeline;
		std::unique_ptr<VertexModel> vertexModel;

		std::vector<VkCommandBuffer> commandBuffers;
		//Pipeline pipeline{ device, Pipeline::defaultCfg(WIDTH, HEIGHT), "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};

		void loadModels();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void recordCommandBuffer(int image_index);
	};
}