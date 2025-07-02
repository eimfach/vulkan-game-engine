#pragma once

#include "vertex_model.hpp"

//std
#include <memory>

namespace Biosim {

	struct Transform2D {
		glm::vec2 translation{}; // position offset
		glm::vec2 scale{ 1.f, 1.f };
		float rotation{};

		glm::mat2 mat2() const {
			//const float s = glm::sin(rotation);
			//const float c = glm::cos(rotation);
			//glm::mat2 rot_matrix{ {c, s}, {-s, c} };

			glm::mat2 scale_mat{{scale.x, .0f}, {.0f, scale.y}};
			return scale_mat;
		}
	};

	class GameObject {
	public:
		using id_t = unsigned int;

		std::shared_ptr<Engine::VertexModel> model{};
		glm::vec3 color{};
		Transform2D transform2D{};

		// delete copy constructor and copy operator
		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;

		// enable move constructor and assignment operator using default
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		static GameObject createGameObject() {
			static id_t current_id = 0;
			return GameObject(current_id++);
		}

		id_t const getId() { return id; }

	private:
		GameObject(id_t obj_id) : id{ obj_id } {}

		id_t id;

	};
}