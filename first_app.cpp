#include "first_app.hpp"

//std
#include <stdexcept>
#include <array>

namespace bm {

	FirstApp::FirstApp() {
		loadModels();
	}

	void FirstApp::run() {
		while (!rendering.windowShouldClose()) {
			glfwPollEvents();
			rendering.drawFrame();
		}

		rendering.deviceWaitIdle();
	}

	void FirstApp::loadModels() {
		float y{ 0.45 };
		verticies = {
			{{0.0f, -y}, {1.0f, 0.0f, 0.0f}},
			{{y, y}, {0.0f, 1.0f, 0.0f}},
			{{-y, y}, {0.0f, 0.0f, 1.0f}},
		};

		rendering.updateVertexModel(verticies);
	}

}