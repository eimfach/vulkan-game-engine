#pragma once

#include "vertex_model.hpp"
#include "device.hpp"
#include "aabb.hpp"

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
#include <array>
#include <iostream>
#include <limits>


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
		glm::mat4 modelMatrix();
		glm::mat3 normalMatrix(glm::mat4& modelMatrix);
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


	///////////////////////////////////////////
	// Entity Managment					     //
	///////////////////////////////////////////

	using EntityId = std::uint16_t;
	using ComponentsMask = std::uint16_t;

	struct Entity {
		ComponentsMask hasComponentsBitmask{};
		EntityId blockId{};
	};

	using RegisteredComponentsStorage = std::tuple<
		std::vector<Identification>,
		std::vector<Visibility>,
		std::vector<Transform>,
		std::vector<PointLight>,
		std::vector<Mesh>,
		std::vector<Color>,
		std::vector<RenderLines>,
		std::vector<AABB>
	>;

	using RegisteredComponentsIndexTable = std::tuple<
		std::pair<Identification, EntityId>,
		std::pair<Visibility, EntityId>,
		std::pair<Transform, EntityId>,
		std::pair<PointLight, EntityId>,
		std::pair<Mesh, EntityId>,
		std::pair<Color, EntityId>,
		std::pair<RenderLines, EntityId>,
		std::pair<AABB, EntityId>
	>;

	/*
	* A ContigiuousComponentsBlock is a block that is determined by its components bitmask (hasComponentsBitmask) and collected in a vector.
	* The entityCount determines how many entites belong to that block which share a component bitmask (exactly the same component types) and where contigiuously added.
	* The offsets determines how each component of an entity is retrieved over all component vectors of GeneralComponentStorage (they have fragmented indicies).
	* The offsets looks at the next_offsets from the previous block and is relavant for retrieving the actual access offsets.
	*/
	struct ContigiuousComponentsBlock {
		EntityId entityCount{};
		ComponentsMask hasComponentsBitmask{};
		ComponentsMask unusedComponentsBitMask{};
		std::array<EntityId, std::tuple_size_v<RegisteredComponentsStorage>> offsets{}; // no offsets can be changed at the moment (copy), after the block was connected with another block 
		std::array<EntityId, std::tuple_size_v<RegisteredComponentsStorage>> next_offsets{};
	};

	class Manager {
	public:
		std::pair<Entity, EntityId> createEntity(bool set_default_components = true);
		void commit(Entity e);
		void lock();

		void reserve_size_entities(size_t size);

		template<typename T> void reserve_size_components(size_t s_components) {
			getComponents<T>().reserve(s_components);
		}

		template<typename T> void addComponent(Entity& entity, T component) {
			std::vector<T>& t_components = std::get<std::vector<T>>(components);
			t_components.emplace_back(component);
			
			entity.hasComponentsBitmask |= 1 << component_type_get_tuple_index<T>();
		}

		template<typename T> std::vector<T>& getComponents() {
			return std::get<std::vector<T>>(components);
		}

		template<typename T> T& getEntityComponent(EntityId id) {
			auto& entity = entities.at(id);
			auto& block = contigiousComponentsBlocks.at(entity.blockId);
			auto& offset = block.offsets.at(component_type_get_tuple_index<T>());
			return getComponents<T>().at(id - offset);
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
		template<typename T> inline EntityId component_type_get_tuple_index() {
			return std::get<std::pair<T, EntityId>>(componentIndex).second;
		}

		template<typename... T> ComponentsMask createComponentsMask() {
			ComponentsMask mask{};
			((mask |= 1 << component_type_get_tuple_index<T>()), ...);
			return mask;
		}

		const EntityId MAX_ENTITIES{ std::numeric_limits<EntityId>::max() };
		EntityId entityCounter{};
		bool commitStage{ false };
		bool locked{ false };
		std::vector<Entity> entities{};
		std::vector<ContigiuousComponentsBlock> contigiousComponentsBlocks{};
		// TODO: Entity can be in multiple groups ?
		std::unordered_map<ComponentsMask, std::vector<EntityId>> entityGroups{};
		RegisteredComponentsStorage components{};
		RegisteredComponentsIndexTable componentIndex{ 
			{{}, 0},
			{{}, 1},
			{{}, 2},
			{{}, 3},
			{{}, 4},
			{{}, 5},
			{{}, 6},
			{{}, 7}
		};

	};
}