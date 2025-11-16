#include "aabb.hpp"

#include "vertex_model.hpp"
#include <array>

namespace nEngine::ECS {

	void AABB::calcuate_min_max(const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform) {
		min = glm::vec3(FLT_MAX);
		max = glm::vec3(-FLT_MAX);

		for (const auto& vertex : verticies) {
			auto position = vertex.position;
			min.x = std::min(min.x, position.x);
			min.y = std::min(min.y, position.y);
			min.z = std::min(min.z, position.z);

			max.x = std::max(max.x, position.x);
			max.y = std::max(max.y, position.y);
			max.z = std::max(max.z, position.z);
		}

		std::array<glm::vec3, 8> edges{
			glm::vec3{ min.x, min.y, min.z },
			glm::vec3{ max.x, min.y, min.z },
			glm::vec3{ min.x, max.y, min.z },
			glm::vec3{ max.x, max.y, min.z },
			glm::vec3{ min.x, min.y, max.z },
			glm::vec3{ max.x, min.y, max.z },
			glm::vec3{ min.x, max.y, max.z },
			glm::vec3{ max.x, max.y, max.z }
		};

		for (size_t i = 0; i < edges.size(); i++) {
			 edges[i] = transform * glm::vec4{edges[i], 1.0f};
		}

		min = glm::vec3(FLT_MAX);
		max = glm::vec3(-FLT_MAX);

		for (const auto& transformed_position : edges) {
			min.x = std::min(min.x, transformed_position.x);
			min.y = std::min(min.y, transformed_position.y);
			min.z = std::min(min.z, transformed_position.z);

			max.x = std::max(max.x, transformed_position.x);
			max.y = std::max(max.y, transformed_position.y);
			max.z = std::max(max.z, transformed_position.z);
		}

		center = (min + max) / 2.f;
		size = max - min;
		halfSize = size / 2.f;

		assert(min.x <= max.x && "AABB min.x must be less than or equal to max.x");
		assert(min.y <= max.y && "AABB min.y must be less than or equal to max.y");
		assert(min.z <= max.z && "AABB min.z must be less than or equal to max.z");
	}
	AABB::AABB(Engine::Device& device, const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform) {

		calcuate_min_max(verticies, transform);

		return;
		
		Engine::VertexModel::Builder model_builder{};

		model_builder.indicies = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
				12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		model_builder.verticies = {
			// Vertices defined by mapping the original cube's coordinates:
			// -0.5f -> min component
			// +0.5f -> max component

			// Left face (white, x = min.x)
			{{min.x, min.y, min.z}, {.9f, .9f, .9f}}, // 0
			{{min.x, max.y, max.z}, {.9f, .9f, .9f}}, // 1
			{{min.x, min.y, max.z}, {.9f, .9f, .9f}}, // 2
			{{min.x, max.y, min.z}, {.9f, .9f, .9f}}, // 3

			// Right face (yellow, x = max.x)
			{{max.x, min.y, min.z}, {.8f, .8f, .1f}}, // 4
			{{max.x, max.y, max.z}, {.8f, .8f, .1f}}, // 5
			{{max.x, min.y, max.z}, {.8f, .8f, .1f}}, // 6
			{{max.x, max.y, min.z}, {.8f, .8f, .1f}}, // 7

			// Top face (orange, y = min.y)
			{{min.x, min.y, min.z}, {.9f, .6f, .1f}}, // 8
			{{max.x, min.y, max.z}, {.9f, .6f, .1f}}, // 9
			{{min.x, min.y, max.z}, {.9f, .6f, .1f}}, // 10
			{{max.x, min.y, min.z}, {.9f, .6f, .1f}}, // 11

			// Bottom face (red, y = max.y)
			{{min.x, max.y, min.z}, {.8f, .1f, .1f}}, // 12
			{{max.x, max.y, max.z}, {.8f, .1f, .1f}}, // 13
			{{min.x, max.y, max.z}, {.8f, .1f, .1f}}, // 14
			{{max.x, max.y, min.z}, {.8f, .1f, .1f}}, // 15

			// Nose face (blue, z = max.z)
			{{min.x, min.y, max.z}, {.1f, .1f, .8f}}, // 16
			{{max.x, max.y, max.z}, {.1f, .1f, .8f}}, // 17
			{{min.x, max.y, max.z}, {.1f, .1f, .8f}}, // 18
			{{max.x, min.y, max.z}, {.1f, .1f, .8f}}, // 19

			// Tail face (green, z = min.z)
			{{min.x, min.y, min.z}, {.1f, .8f, .1f}}, // 20
			{{max.x, max.y, min.z}, {.1f, .8f, .1f}}, // 21
			{{min.x, max.y, min.z}, {.1f, .8f, .1f}}, // 22
			{{max.x, min.y, min.z}, {.1f, .8f, .1f}}, // 23
		};

		// TODO: Crashes occasionally
		//model = std::make_shared<Engine::VertexModel>(device, model_builder);
	}

	AABB::AABB(const std::vector<Engine::VertexBase>& verticies, const glm::mat4& transform) {
		calcuate_min_max(verticies, transform);
	}

	bool AABB::intersects(const AABB& aabb) const {
		return (
			max.x >= aabb.min.x &&
			max.y >= aabb.min.y &&
			max.z >= aabb.min.z &&
			min.x <= aabb.max.x &&
			min.y <= aabb.max.y &&
			min.z <= aabb.max.z
			);
		//return glm::all(glm::greaterThan(max, aabb.min) && glm::lessThan(min, max));
	}
}