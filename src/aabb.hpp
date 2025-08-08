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
	const std::vector<Engine::VertexBase> voxelVerticies{ // represents a cube
		// left face (white)
		{{-.5f, -.5f, -.5f}},
		{{-.5f, .5f, .5f}},
		{{-.5f, -.5f, .5f}},
		{{-.5f, .5f, -.5f}},

		// right face (yellow)
		{{.5f, -.5f, -.5f}},
		{{.5f, .5f, .5f}},
		{{.5f, -.5f, .5f}},
		{{.5f, .5f, -.5f}},

		// top face (orange, remember y axis points down)
		{{-.5f, -.5f, -.5f}},
		{{.5f, -.5f, .5f}},
		{{-.5f, -.5f, .5f}},
		{{.5f, -.5f, -.5f}},

		// bottom face (red)
		{{-.5f, .5f, -.5f}},
		{{.5f, .5f, .5f}},
		{{-.5f, .5f, .5f}},
		{{.5f, .5f, -.5f}},

		// nose face (blue)
		{{-.5f, -.5f, 0.5f}},
		{{.5f, .5f, 0.5f}},
		{{-.5f, .5f, 0.5f}},
		{{.5f, -.5f, 0.5f}},

		// tail face (green)
		{{-.5f, -.5f, -0.5f}},
		{{.5f, .5f, -0.5f}},
		{{-.5f, .5f, -0.5f}},
		{{.5f, -.5f, -0.5f}},
	};

	struct AABB {
		glm::vec3 min{};
		glm::vec3 max{};
		std::shared_ptr<Engine::VertexModel> model = nullptr;

		AABB(Engine::Device& device, const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform);
		AABB(const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform);
		AABB() = default;
		//AABB(const AABB&) = delete;
		//AABB& operator= (const AABB&) = delete;

		void inline calcuateMinMax(const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform);
		virtual bool intersects(const AABB& aabb) const;
	};

	struct Voxel : AABB {
		Voxel(Engine::Device& device, const glm::mat4& transform);
		Voxel(const glm::mat4& transform);
		Voxel() = default;
	};
}
