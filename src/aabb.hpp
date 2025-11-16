#pragma once

#include "vertex_model.hpp"
#include "device.hpp"
#include "vertex_base.hpp"

// libs
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace nEngine::ECS {
	struct AABB {
		glm::vec3 min{};
		glm::vec3 max{};
		glm::vec3 center{};
		glm::vec3 size{};
		glm::vec3 halfSize{};
		std::shared_ptr<Engine::VertexModel> model = nullptr;

		AABB(Engine::Device& device, const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform);
		AABB(const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform);
		AABB() = default;
		//AABB(const AABB&) = delete;
		//AABB& operator= (const AABB&) = delete;

		void inline calcuate_min_max(const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform);
		virtual bool intersects(const AABB& aabb) const;
	};
}
