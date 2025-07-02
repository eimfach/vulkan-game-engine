#pragma once

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

		// delete copy constructor and copy operator
		FirstApp(const FirstApp&) = delete;
		FirstApp& operator= (const FirstApp&) = delete;

		void run();

	private:
		Engine::Renderer rendering{};
		std::vector<GameObject> gameObjects;

		void loadGameObjects();
	};
}