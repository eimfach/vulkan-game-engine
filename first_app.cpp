#include "first_app.hpp"

#include <gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>

namespace Biosim {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	void FirstApp::run() {
		while (!rendering.windowShouldClose()) {
			glfwPollEvents();
			for (auto& obj : gameObjects) {
				obj.transform2D.rotation = glm::mod(obj.transform2D.rotation + 0.01f, glm::two_pi<float>());
			}
			rendering.drawFrame(gameObjects);
		}

		rendering.deviceWaitIdle();
	}

	void FirstApp::loadGameObjects() {
		float y{ 0.45 };
		std::vector<Engine::VertexBase> verticies = {
			{{0.0f, -y}, {1.0f, 0.0f, 0.0f}},
			{{y, y}, {0.0f, 1.0f, 0.0f}},
			{{-y, y}, {0.0f, 0.0f, 1.0f}},
		};

		auto vertex_model = rendering.createVertexModel(verticies);
		auto triangle = GameObject::createGameObject();
		triangle.model = vertex_model;
		triangle.color = { .2f, .8f, .2f };
		triangle.transform2D.translation.x = .2f;
		triangle.transform2D.scale = { 2.f, .5f };
		triangle.transform2D.rotation = .25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}

}