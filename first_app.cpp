#include "first_app.hpp"

#include "simple_render_system.hpp"
#include "camera.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>
#include <gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>
#include <cassert>

namespace Biosim {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {}

	void FirstApp::run() {
		Engine::SimpleRenderSystem render_system{ device, renderer.getSwapChainRenderPass() };
		Engine::Camera camera{};
		/*camera.setViewDirection(glm::vec3{ 0.f }, glm::vec3(0.5f, 0.f, 1.f));*/
		camera.setViewTarget(glm::vec3{ -1.f, -2.f, -5.f }, glm::vec3{0.f, 0.f, 2.5f});

		while (!renderer.windowShouldClose()) {
			glfwPollEvents();

			float aspect = renderer.getAspectRatio();
			//camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 20.f);

			for (auto& obj : gameObjects) {
				obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
				obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());
			}

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
		auto cube_model = renderer.createCubeModel({.0f, .0f, .0f});
		auto cube = GameObject::createGameObject();
		cube.model = cube_model;
		cube.transform.translation = { 0.f, 0.f, 2.5f };
		cube.transform.scale = { .5f, .5f, .5f };
		gameObjects.push_back(std::move(cube));
	}
}