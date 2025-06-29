#pragma once

#include "window.hpp"
#include "rendering.hpp"
#include "vertex_base.hpp"

// std
#include <memory>
#include <vector>

namespace bm {
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
		Rendering rendering{};
		std::vector<VertexBase> verticies;

		void loadModels();
	};
}