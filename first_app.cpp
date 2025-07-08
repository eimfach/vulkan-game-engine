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

namespace SJFGame {
	// check alignment rules before making changes to buffer object structures
	// https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/app09lev1sec2.html
	struct GlobalUniformBufferOutput {
		glm::mat4 projectionView{ 1.f };

		// vec3 and vec4 needs to be aligned to 16 Bytesz
		// Options: 
		// 1. Place a padding member inbetween (tight packing by the host, cpu/gpu layout matching)
		// vec3 A;
		// uint32_t padding;
		// vec4 B;
		// 2. Instead of vec3 use a vec4 ignoring the w component 
		// (avoiding any bugs in the external implementation; 
		// Should not be a issue with Vulkan because Shaders are compiled to SPIR-V)
		// 3. Use alignas(16) to fill up space between vec3 and vec4
		glm::vec3 directionalLightPosition = { 4.f, -3.f, -1.f };
		alignas(16) glm::vec4 directionalLightColor = { 0.f, 1.0f, .3f, 0.f }; // w is intensity
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .05f }; // w is intensity
		glm::vec3 positionalLightPosition{ -1.f };
		alignas(16) glm::vec4 positionalLightColor{ 1.f, 0.f, 0.f, 1.f }; // w is light intensity
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
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
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
		//camera.setViewTarget(glm::vec3{ -1.f, -2.f, -5.f }, glm::vec3{ .5f, .5f, 0.f });
		auto camera_state = GameObject::createGameObject();
		camera_state.transform.translation.z = -2.5f;
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
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 100.f);

			if (auto cmd_buffer = renderer.beginFrame()) {
				int frame_index = renderer.getFrameIndex();
				Engine::Frame frame{frame_index, frame_delta_time, camera, cmd_buffer, global_descriptor_sets[frame_index], gameObjects};

				// update 
				GlobalUniformBufferOutput ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				ubo_buffers[frame_index]->writeToBuffer(&ubo);
				ubo_buffers[frame_index]->flush();

				// render
				renderer.beginSwapChainRenderPass(cmd_buffer);
				render_system.renderObjects(frame);
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
		obj.transform.translation = { -.5f, .5f, 0.f };
		obj.transform.scale = glm::vec3{ 3.f, 1.5f, 3.f };

		auto model1 = Engine::VertexModel::createModelFromFile(device, "models/smooth_vase.obj");
		auto obj1 = GameObject::createGameObject();
		obj1.model = model1;
		obj1.transform.translation = { .5f, .5f, 0.f };
		obj1.transform.scale = glm::vec3{ 3.f, 1.5f, 3.f };

		auto plane_model = Engine::VertexModel::createModelFromFile(device, "models/quad.obj");
		auto plane_obj = GameObject::createGameObject();
		plane_obj.model = plane_model;
		plane_obj.transform.translation = { .0f, .5f, 0.f };
		plane_obj.transform.scale = glm::vec3{ 3.f, 1.f, 3.f };

		gameObjects.emplace(obj.getId(), std::move(obj));
		gameObjects.emplace(obj1.getId(), std::move(obj1));
		gameObjects.emplace(plane_obj.getId(), std::move(plane_obj));
	}
}