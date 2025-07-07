#include "first_app.hpp"

#include "buffer.hpp"
#include "camera.hpp"
#include "movement.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <chrono>
#include <ratio>

namespace Biosim {
	struct GlobalUniformBufferOutput {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 4.f, -3.f, -1.f });
	};

	FirstApp::FirstApp() {
		globalPool = Engine::DescriptorPool::Builder(device)
			.setMaxSets(Engine::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Engine::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		loadGameObjects();
	}

	FirstApp::~FirstApp() {
		globalPool = nullptr;
	}

	void FirstApp::run() {
		std::vector<std::unique_ptr<Engine::Buffer>> ubo_buffers(Engine::SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < ubo_buffers.size(); i++)
		{
			ubo_buffers[i] = std::make_unique<Engine::Buffer>(
				device,
				sizeof(GlobalUniformBufferOutput),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			ubo_buffers[i]->map();
		}

		auto global_set_layout = Engine::DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		std::vector<VkDescriptorSet> global_descriptor_sets(Engine::SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (size_t i = 0; i < global_descriptor_sets.size(); i++) {
			auto buffer_info = ubo_buffers[i]->descriptorInfo();
			Engine::DescriptorWriter(*global_set_layout, *globalPool)
				.writeBuffer(0, &buffer_info)
				.build(global_descriptor_sets[i]);
		}
		// -> uniform buffers and descriptors could be managed in a Master Render System

		Engine::SimpleRenderSystem render_system{ device, renderer.getSwapChainRenderPass(), global_set_layout->getDescriptorSetLayout()};
		
		Engine::Camera camera{};
		camera.setViewTarget(glm::vec3{ -1.f, -2.f, -5.f }, glm::vec3{ .5f, .5f, 2.5f });
		auto camera_state = GameObject::createGameObject();
		Engine::MovementControl camera_control{};

		auto current_time = std::chrono::high_resolution_clock::now();

		while (!renderer.windowShouldClose()) {
			glfwPollEvents();

			auto new_time = std::chrono::high_resolution_clock::now();
			float frame_delta_time = std::chrono::duration<float, std::ratio<1>>(new_time - current_time).count();
			current_time = new_time;
			//frame_delta_time = glm::min(frame_delta_time, MAX_FRAME_TIME);

			camera_control.moveInPlaneXZ(window.getGLFWwindow(), frame_delta_time, camera_state);
			camera.setViewYXZ(camera_state.transform.translation, camera_state.transform.rotation);
			float aspect = renderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 20.f);

			if (auto cmd_buffer = renderer.beginFrame()) {
				int frame_index = renderer.getFrameIndex();
				Engine::Frame frame{frame_index, frame_delta_time, camera, cmd_buffer, global_descriptor_sets[frame_index]};

				// update 
				GlobalUniformBufferOutput ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				ubo_buffers[frame_index]->writeToBuffer(&ubo);
				ubo_buffers[frame_index]->flush();

				// render
				renderer.beginSwapChainRenderPass(cmd_buffer);
				render_system.renderObjects(gameObjects, frame);
				renderer.endSwapChainRenderPass(cmd_buffer);
				renderer.endFrame();
			}
		}

		renderer.deviceWaitIdle();
	}

	void FirstApp::loadGameObjects() {
		//auto cube_model = renderer.createCubeModel({.0f, .0f, .0f});
		auto model = Engine::VertexModel::createModelFromFile(device, "models/flat_vase.obj");
		auto obj = GameObject::createGameObject();
		obj.model = model;
		obj.transform.translation = { .5f, .5f, 2.5f };
		obj.transform.scale = glm::vec3{ 3.f, 1.5f, 3.f };

		auto model1 = Engine::VertexModel::createModelFromFile(device, "models/smooth_vase.obj");
		auto obj1 = GameObject::createGameObject();
		obj1.model = model1;
		obj1.transform.translation = { -.5f, .5f, 2.5f };
		obj1.transform.scale = glm::vec3{ 3.f, 1.5f, 3.f };

		gameObjects.push_back(std::move(obj));
		gameObjects.push_back(std::move(obj1));
	}
}