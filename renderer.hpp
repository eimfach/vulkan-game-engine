#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "vertex_model.hpp"
#include "vertex_base.hpp"
#include "game_object.hpp"

// std
#include <memory>
#include <vector>

namespace Biosim::Engine {
	class Renderer {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		Renderer();
		~Renderer();

		// delete copy constructor and copy operator
		Renderer(const Renderer&) = delete;
		Renderer& operator= (const Renderer&) = delete;

		void drawFrame(const std::vector<GameObject>& objects);
		void deviceWaitIdle();
		bool windowShouldClose();
		std::shared_ptr<Biosim::Engine::VertexModel> createVertexModel(const std::vector<VertexBase>& verticies);

	private:
		Window window{ WIDTH, HEIGHT, "Hello Vulkan!" };
		Device device{ window };
		VkPipelineLayout pipelineLayout;

		std::unique_ptr<SwapChain> swapChain;
		std::unique_ptr<Pipeline> pipeline;
		//std::unique_ptr<VertexModel> vertexModel;

		std::vector<VkCommandBuffer> commandBuffers;
		//Pipeline pipeline{ device, Pipeline::defaultCfg(WIDTH, HEIGHT), "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};

		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void recordCommandBuffer(const std::vector<GameObject>& objects, int image_index);
		void renderObjects(const std::vector<GameObject>& objects, VkCommandBuffer cmd_buffer);
	};
}