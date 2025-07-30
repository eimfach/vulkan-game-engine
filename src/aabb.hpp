#pragma once

#include "vertex_model.hpp"
#include "device.hpp"
#include "vertex_base.hpp"

// libs
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace SJFGame::ECS {
	struct AABB {
		glm::vec3 min{};
		glm::vec3 max{};
		std::shared_ptr<Engine::VertexModel> model = nullptr;

		AABB(const std::vector<Engine::VertexBase>& verticies, Engine::Device& device);
		AABB() = default;
		bool inline intersects(AABB aabb) const;
	};
}
