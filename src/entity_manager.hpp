#pragma once

#include "vertex_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <cstdint>
#include <string>
#include <memory>
#include <tuple>
#include <cassert>
#include <typeinfo>
#include <utility>
#include <vector>
#include <unordered_map>

namespace SJFGame::ECS {

	struct Identification {
		std::string name = "name_not_set";
	};

	enum RenderProperty {
		RENDER_DEFAULT = 0,
		RENDER_AS_LINES = 1,
	};

	struct RenderProperties {
		uint8_t drawMode = RENDER_DEFAULT;
		bool visible = true;
	};

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

	struct PointLight {
		float lightIntensity{ 1.0f };
	};

	struct Mesh {
		std::shared_ptr<Engine::VertexModel> model{};
	};

	struct Color {
		glm::vec3 rgb = { 1.f, 1.f, 1.f };
	};

	using EntityId = std::uint16_t;
	using ComponentsMask = std::uint16_t;

	struct Entity {
		EntityId id{};
		ComponentsMask has_components_in{};
	};

	using ComponentStorage = std::tuple<
		std::vector<Identification>,
		std::vector<RenderProperties>,
		std::vector<Transform>,
		std::vector<PointLight>,
		std::vector<Mesh>,
		std::vector<Color>
	>;

	using ComponentStorageIndex = std::tuple<
		std::pair<Identification, EntityId>,
		std::pair<RenderProperties, EntityId>,
		std::pair<Transform, EntityId>,
		std::pair<PointLight, EntityId>,
		std::pair<Mesh, EntityId>,
		std::vector<Color, EntityId>
	>;

	class Manager {
	public:
		Entity createEntity();
		void commit(Entity e);
		std::vector<EntityId>& getGroup(ComponentsMask components_mask);

		template<typename T> void addComponent(Entity& entity, T component) {
			std::vector<T>& t_components = std::get<std::vector<T>>(components);
			t_components.push_back(component);
			
			EntityId index = std::get<std::pair<T, EntityId>>(componentIndex).second;
			entity.has_components_in |= 1 << index;
		}

		template<typename T> EntityId entity_type_get_tuple_index() {
			return std::get<std::pair<T, EntityId>>(componentIndex).second;
		}

		template<typename... T> ComponentsMask createComponentsMask() {
			ComponentsMask mask{};
			((mask |= 1 << entity_type_get_tuple_index<T>()), ...);
			return mask;
		}

		template<typename T> std::vector<T>& getComponents() {
			return std::get<std::vector<T>>(components);
		}

		// delete copy constructor and copy operator
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;

		Manager();
		~Manager();

	private:
		const EntityId MAX_ENTITIES{65000};
		EntityId counter{};
		bool commitStage{false};
		std::unordered_map<ComponentsMask, std::vector<EntityId>> entityGroups{};
		ComponentStorage components{};
		const ComponentStorageIndex componentIndex{ 
			{{}, 0},
			{{}, 1},
			{{}, 2},
			{{}, 3},
			{{}, 4},
			{{}, 5}
		};

	};
}