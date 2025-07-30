#include "aabb.hpp"

namespace SJFGame::ECS {

	AABB::AABB(const std::vector<Engine::VertexBase>& verticies, Engine::Device& device) {
		for (auto& vertex : verticies) {
			auto position = vertex.position;
			if (position.z > min.z) {
				min.z = position.z;
			}
			if (position.y > min.y) {
				min.y = position.y;
			}
			if (position.x < min.x) {
				min.x = position.x;
			}

			if (position.z < max.z) {
				max.z = position.z;
			}
			if (position.y < max.y) {
				max.y = position.y;
			}
			if (position.x > max.x) {
				max.x = position.x;
			}

		}

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

		//for (auto& v : model_builder.verticies) {
		//	v.position += offset;
		//}
		model = std::make_shared<Engine::VertexModel>(device, model_builder);
	}

	bool AABB::intersects(AABB aabb) const {
		return glm::all(glm::greaterThan(max, aabb.min) && glm::lessThan(min, max));
	}
}