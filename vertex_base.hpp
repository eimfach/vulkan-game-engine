#pragma once

#include <glm.hpp>

namespace Biosim::Engine {
	struct VertexBase {
		glm::vec3 position{};
		glm::vec3 color{};
		glm::vec3 normal{};
		glm::vec2 uv{};

		bool operator==(const VertexBase& v) const {
			return position == v.position &&
				color == v.color &&
				normal == v.normal &&
				uv == v.uv;
		}
	};
}