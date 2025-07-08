#pragma once

#include "vertex_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>
//std
#include <memory>
#include <unordered_map>

namespace SJFGame {

	struct Transform {
		glm::vec3 translation{}; // position offset
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corresponds to: translate * Ry * Rx * Rz * scale transformation.
		// Rotation convention uses trait-bryan angles with axis order Y(1), X(2), Z(3)
		//glm::mat4 mat4() const {
		//	auto transform = glm::translate(glm::mat4{ 1.f }, translation);
		//	transform = glm::rotate(transform, rotation.y, { 0.f, 1.f, 0.f });
		//	transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
		//	transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });
		//	transform = glm::scale(transform, scale);
		//	return transform;
		//}

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4() const;

		// this may be slow with a lot if game objects (can use glm to calculate inverse transpose of the model matrix)
		glm::mat3 normalMatrix() const;
	};

	class GameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, GameObject>;

		std::shared_ptr<Engine::VertexModel> model{};
		glm::vec3 color{};
		Transform transform{};

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
		id_t id;

		GameObject(id_t obj_id) : id{ obj_id } {}


	};
}