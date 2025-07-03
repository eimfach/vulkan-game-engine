#pragma once

#include "device.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "vertex_base.hpp"
#include "game_object.hpp"

// std
#include <memory>
#include <vector>

namespace Biosim {
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
		Engine::Window window{ WIDTH, HEIGHT, "Hello Vulkan!"};
		Engine::Device device{ window };
		Engine::Renderer renderer{window, device};

		std::vector<GameObject> gameObjects;

		void loadGameObjects();
	};
}