#include "first_app.hpp"

#include "movement.hpp"
#include "simple_render_system.hpp"
#include "camera.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <cassert>
#include <chrono>
#include <ratio>
#include <iostream>

namespace Biosim {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		Engine::SimpleRenderSystem render_system{ device, renderer.getSwapChainRenderPass() };
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
				renderer.beginSwapChainRenderPass(cmd_buffer);
				render_system.renderObjects(gameObjects, cmd_buffer, camera);
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