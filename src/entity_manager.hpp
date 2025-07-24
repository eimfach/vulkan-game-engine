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

	///////////////////////////////////////////
	// Components							 //
	///////////////////////////////////////////

	struct Identification {
		std::string name = "name_not_set";
	};

	enum RenderProperty {
		RENDER_DEFAULT = 0,
		RENDER_AS_LINES = 1,
	};

	struct Visibility {
		bool visible{true};
	};

	struct RenderLines {};

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
		std::shared_ptr<Engine::VertexModel> model = nullptr;
	};

	struct Color {
		glm::vec3 rgb{ 1.f, 1.f, 1.f };
	};

	struct AABB {
		glm::vec3 min{};
		glm::vec3 max{};

		AABB(std::vector<glm::vec3> verticies);
		AABB() = default;
	};

	///////////////////////////////////////////
	// Entity Managment					     //
	///////////////////////////////////////////

	using EntityId = std::uint16_t;
	using ComponentsMask = std::uint16_t;

	struct Entity {
		EntityId id{};
		ComponentsMask hasComponentsBitmask{};
	};

	using GeneralComponentStorage = std::tuple<
		std::vector<Identification>,
		std::vector<Visibility>,
		std::vector<Transform>,
		std::vector<PointLight>,
		std::vector<Mesh>,
		std::vector<Color>,
		std::vector<RenderLines>,
		std::vector<AABB>
	>;

	using GeneralComponentStorageIndex = std::tuple<
		std::pair<Identification, EntityId>,
		std::pair<Visibility, EntityId>,
		std::pair<Transform, EntityId>,
		std::pair<PointLight, EntityId>,
		std::pair<Mesh, EntityId>,
		std::pair<Color, EntityId>,
		std::pair<RenderLines, EntityId>,
		std::pair<AABB, EntityId>
	>;

	class Manager {
	public:
		Entity createEntity(bool set_default_components = true);
		void commit(Entity e);

		template<typename T> void addComponent(Entity& entity, T component) {
			std::vector<T>& t_components = std::get<std::vector<T>>(components);
			t_components.push_back(component);
			
			entity.hasComponentsBitmask |= 1 << entity_type_get_tuple_index<T>();
		}

		template<typename T> std::vector<T>& getComponents() {
			return std::get<std::vector<T>>(components);
		}

		template<typename T> T& getEntityComponent(EntityId id) {
			return getComponents<T>().at(id);
		}

		template<typename... T> bool hasEntityComponents(EntityId id) {
			ComponentsMask requested_mask = createComponentsMask<T...>();
			return ((requested_mask & entities[id].hasComponentsBitmask) == requested_mask);
		}

		template <typename... T> std::vector<EntityId>& getEntityGroup() {
			ComponentsMask mask = createComponentsMask<T...>();
			assert((entityGroups.count(mask) > 0) && "Requested Component Group does not exist!");
			return entityGroups.at(mask);
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
		std::vector<Entity> entities{};
		// TODO: Entity can be in multiple groups ?
		std::unordered_map<ComponentsMask, std::vector<EntityId>> entityGroups{};
		GeneralComponentStorage components{};
		GeneralComponentStorageIndex componentIndex{ 
			{{}, 0},
			{{}, 1},
			{{}, 2},
			{{}, 3},
			{{}, 4},
			{{}, 5},
			{{}, 6},
			{{}, 7},
		};

		template<typename T> EntityId entity_type_get_tuple_index() {
			return std::get<std::pair<T, EntityId>>(componentIndex).second;
		}

		template<typename... T> ComponentsMask createComponentsMask() {
			ComponentsMask mask{};
			((mask |= 1 << entity_type_get_tuple_index<T>()), ...);
			return mask;
		}
	};
}