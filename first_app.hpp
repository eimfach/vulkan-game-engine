#pragma once

#include "device.hpp"
#include "window.hpp"
#include "renderer.hpp"
#include "vertex_base.hpp"
#include "game_object.hpp"
#include "descriptors.hpp"

// std
#include <memory>

namespace SJFGame {
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

		// note: order of declaration matters (not if globalPool gets descructed in ~FirstApp which is the case; the pools need to be destroyed before the device)
		std::unique_ptr<Engine::DescriptorPool> globalPool{};
		GameObject::Map gameObjects;

		void loadGameObjects();
	};
}