#pragma once

#include "device.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "vertex_base.hpp"
#include "game_object.hpp"
#include "descriptors.hpp"
#include "entity_manager.hpp"

// std
#include <future>
#include <vector>

namespace nEngine {
	class FirstApp {
	public:
		static constexpr int WIDTH = 1024;
		static constexpr int HEIGHT = 768;

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

		GameObject::Map gameObjects;
		ECS::EntityId viewerId{};
		ECS::Manager ecsManager{};

		std::vector<std::future<void>> futures;

		void loadGameEntities();
		void loadLineEntities(const int count);
		void loadPointLightEntities();
		void loadStaticObjects();
		void loadViewer();
	};
}