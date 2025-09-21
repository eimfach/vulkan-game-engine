#pragma once

#include "vertex_model.hpp"
#include "device.hpp"
#include "aabb.hpp"
#include "buffered_vector.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

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
#include <mutex>


namespace nEngine::ECS {

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

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		inline glm::mat4 modelMatrix() {
			// Create quaternion from Tait-Bryan angles (y, x, z order)
			glm::quat rotation_quat = glm::quat(rotation);

			// Create model matrix
			glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), translation)
				* glm::mat4_cast(rotation_quat)
				* glm::scale(glm::mat4(1.0f), scale);

			return model_mat;

			//const float c3 = glm::cos(rotation.z);
			//const float s3 = glm::sin(rotation.z);
			//const float c2 = glm::cos(rotation.x);
			//const float s2 = glm::sin(rotation.x);
			//const float c1 = glm::cos(rotation.y);
			//const float s1 = glm::sin(rotation.y);
			//return glm::mat4{
			//	{
			//		scale.x * (c1 * c3 + s1 * s2 * s3),
			//		scale.x * (c2 * s3),
			//		scale.x * (c1 * s2 * s3 - c3 * s1),
			//		0.0f,
			//	},
			//	{
			//		scale.y * (c3 * s1 * s2 - c1 * s3),
			//		scale.y * (c2 * c3),
			//		scale.y * (c1 * c3 * s2 + s1 * s3),
			//		0.0f,
			//	},
			//	{
			//		scale.z * (c2 * s1),
			//		scale.z * (-s2),
			//		scale.z * (c1 * c2),
			//		0.0f,
			//	},
			//	{
			//		translation.x,
			//		translation.y,
			//		translation.z,
			//		1.0f
			//	}
			//};
		}

		inline glm::mat3 normalMatrix(glm::mat4& model_matrix) {
			return glm::transpose(glm::inverse(glm::mat3(model_matrix)));
			//const float c3 = glm::cos(rotation.z);
			//const float s3 = glm::sin(rotation.z);
			//const float c2 = glm::cos(rotation.x);
			//const float s2 = glm::sin(rotation.x);
			//const float c1 = glm::cos(rotation.y);
			//const float s1 = glm::sin(rotation.y);
			//const glm::vec3 inv_scale = 1.0f / scale;
			//return glm::mat3{
			//	{
			//		inv_scale.x * (c1 * c3 + s1 * s2 * s3),
			//		inv_scale.x * (c2 * s3),
			//		inv_scale.x * (c1 * s2 * s3 - c3 * s1),
			//	},
			//	{
			//		inv_scale.y * (c3 * s1 * s2 - c1 * s3),
			//		inv_scale.y * (c2 * c3),
			//		inv_scale.y * (c1 * c3 * s2 + s1 * s3),
			//	},
			//	{
			//		inv_scale.z * (c2 * s1),
			//		inv_scale.z * (-s2),
			//		inv_scale.z * (c1 * c2),
			//	},
			//};
		}
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
		bool merge{ false };

		ContigiuousComponentsBlock& operator+=(ContigiuousComponentsBlock& rhs) {
			entityCount += 1;
			for (int i{}; i < next_offsets.size(); i++) {
				if (next_offsets[i] == 0) {
					continue;
				}
				next_offsets[i] += 1;
			}

			return *this;
		}
	};

	constexpr std::make_index_sequence<std::tuple_size_v<RegisteredComponentsStorage>> COMPONENTS_INDEX_SEQUENCE{};




	/*///////////////////////////////////////////
	  // Archetype ECS System                  //
	  ///////////////////////////////////////////

	  Entity Component System Manager
	  This manager contains entities (think game objects) and each entity can build from different components (Like Transform, Color, Mesh etc.).
	  The components are stored in a vector for each component type.
	*/
	class Manager {
	public:
		BufferedVector<ECS::Transform> bf{}; // testing

		std::pair<Entity, EntityId> createEntity(bool set_default_components = true);
		void commit(Entity e);
		void lock();

		template<typename T> void addComponent(Entity& entity, T component) {
			assert((entity.hasComponentsBitmask & createComponentsMask<T>()) == 0 && "Entity already has this component!");
			// 1. t_components.pushBuffer(component);
			std::vector<T>& t_components = std::get<std::vector<T>>(components);
			t_components.emplace_back(component);

			entity.hasComponentsBitmask |= 1 << componentTypeGetTupleIndex<T>();
		}

		// https://www.cppstories.com/2022/tuple-iteration-basics/
		template <std::size_t... Is> void syncBuffers(std::index_sequence<Is...>) {
			if (commitStage) {
				return;
			}

			bf.syncElement();
			// 2a. entities.sync_element();
			// 2b. contigiousComponentsBlocks.sync_element();
			// 2c. std::get<BufferedVector<Transform>>(components).sync_element();
			// 2c. (std::get<Is>(components).sync_element(), ...);
		}

		void reserveSizeEntities(size_t size);

		template<typename T> void reserveSizeComponents(size_t s_components) {
			getComponents<T>().reserve(s_components);
		}

		template<typename T> std::vector<T>& getComponents() {
			// 3. std::get<BufferedVector<T>>(components)
			return std::get<std::vector<T>>(components);
		}

		template<typename T> T& getEntityComponent(EntityId id) {
			auto& entity = entities.at(id);
			auto& block = contigiousComponentsBlocks.at(entity.blockId);
			auto& offset = block.offsets.at(componentTypeGetTupleIndex<T>());
			return getComponents<T>().at(id - offset);
		}

		template<typename... T> bool hasEntityComponents(EntityId id) {
			ComponentsMask requested_mask = createComponentsMask<T...>();
			return ((requested_mask & entities[id].hasComponentsBitmask) == requested_mask);
		}

		template <typename... T> std::vector<EntityId>& getEntityGroup() {
			ComponentsMask mask = createComponentsMask<T...>();
			//assert((entityGroups.count(mask) > 0) && "Requested Component Group does not exist!");
			return entityGroups.at(mask);
		}

		// delete copy constructor and copy operator
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;

		Manager();
		~Manager();

	private:
		template<typename T> inline EntityId componentTypeGetTupleIndex() {
			return std::get<std::pair<T, EntityId>>(componentIndex).second;
		}

		template<typename... T> ComponentsMask createComponentsMask() {
			ComponentsMask mask{};
			((mask |= 1 << componentTypeGetTupleIndex<T>()), ...);
			return mask;
		}

		const EntityId MAX_ENTITIES{ std::numeric_limits<EntityId>::max() };
		EntityId entityCounter{};
		bool commitStage{ false };
		bool locked{ false };
		std::vector<Entity> entities{}; // TODO: BufferedVector
		std::vector<ContigiuousComponentsBlock> contigiousComponentsBlocks{}; // TODO: BufferedVector
		// 4b. Entity can be in multiple groups: Use enum for groups and on commit, 
		// give a variable list of enums. Init each group as empty vector. 
		// The groups member should be multiple BufferedVector's accordingly.
		// TODO: reserve group vector boundaries
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