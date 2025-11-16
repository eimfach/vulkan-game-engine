#pragma once

#include "entity_types.hpp"

// libs

// std
#include <array>
#include <cassert>
#include <limits>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>



namespace nEngine::ECS {

	///////////////////////////////////////////
	// Entity Managment					     //
	///////////////////////////////////////////

	constexpr std::make_index_sequence<std::tuple_size_v<RegisteredComponentsStorage>> COMPONENTS_INDEX_SEQUENCE{};

	enum class Groups {
		camera,
		simple_render,
		pointlight_render,
		line_render,
		gui_stats,

		size,
	};

	/*///////////////////////////////////////////
	  // Archetype ECS System                  //
	  ///////////////////////////////////////////

	  Entity Component System Manager
	  This manager contains entities (think game objects) and each entity can build from different components (Like Transform, Color, Mesh etc.).
	  The components are stored in a vector for each component type.
	*/
	class Manager {
	public:
		bool sync_in_progress{ true };
		std::pair<Entity, EntityId> createEntity(bool set_default_components = true);
		void commit(Entity e, std::vector<Groups>& groups);
		void lock();
		std::vector<EntityId>& getEntityGroup(Groups group);

		// delete copy constructor and copy operator
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager&) = delete;

		Manager() {};
		~Manager() {};

		template<typename T>
		void addComponent(Entity& entity, T component) {
			sync_in_progress = true;
			assert((entity.hasComponentsBitmask & createComponentsMask<T>()) == 0 && "Entity already has this component!");
			std::get<BufferedVector<T>>(components).pushBuffer(component);

			entity.hasComponentsBitmask |= 1 << componentTypeGetTupleIndex<T>();
		}

		// https://www.cppstories.com/2022/tuple-iteration-basics/
		template <std::size_t... Is>
		void syncBuffers(std::index_sequence<Is...>) {
			if (commitStage) {
				return;
			}

			bool buffers_filled{ false };
			
			buffers_filled |= entities.syncElement();
			buffers_filled |= contigiousComponentsBlocks.syncElement();

			for (int i = 0; i < entityGroups.size(); i++) {
				buffers_filled |= entityGroups[i].syncElement();
			}
			
			((buffers_filled |= std::get<Is>(components).syncElement()), ...);

			sync_in_progress = buffers_filled;
		}

		void reserveSizeEntities(size_t size);

		template<typename T>
		void reserveSizeComponents(size_t s_components) {
			getComponents<T>().reserve(s_components);
		}

		template<typename T>
		std::vector<T>& getComponents() {
			return std::get<BufferedVector<T>>(components).getWriteable();
		}

		template<typename T>
		T& getEntityComponent(EntityId id) {
			assert(id < entities.getWriteable().size() && "ECS::Manager::getEntityComponent Out of bounds access to entities vector");
			Entity& entity = entities.getWriteable()[id];

			assert(entity.blockId < contigiousComponentsBlocks.getWriteable().size() && "ECS::Manager::getEntityComponent Out of bounds access to contigiousComponentsBlocks vector");
			ContiguousComponentsBlock& block = contigiousComponentsBlocks.getWriteable()[entity.blockId];

			assert(componentTypeGetTupleIndex<T>() < block.offsets.size() && "ECS::Manager::getEntityComponent Out of bounds access to block.offsets array");
			EntityId& offset = block.offsets[componentTypeGetTupleIndex<T>()];

			return getComponents<T>()[id - offset];
		}

		template<typename... T>
		bool hasEntityComponents(EntityId id) {
			ComponentsMask requested_mask = createComponentsMask<T...>();
			return ((requested_mask & entities[id].hasComponentsBitmask) == requested_mask);
		}


	private:
		template<typename T>
		inline EntityId componentTypeGetTupleIndex() {
			return std::get<std::pair<T, EntityId>>(componentIndex).second;
		}

		template<typename... T>
		ComponentsMask createComponentsMask() {
			ComponentsMask mask{};
			((mask |= 1 << componentTypeGetTupleIndex<T>()), ...);
			return mask;
		}

		const EntityId MAX_ENTITIES{ std::numeric_limits<EntityId>::max() };
		EntityId entityCounter{};
		bool commitStage{ false };
		bool locked{ false };
		BufferedVector<Entity> entities{};
		BufferedVector<ContiguousComponentsBlock> contigiousComponentsBlocks{};
		// TODO: reserve group vector boundaries
		std::array<BufferedVector<EntityId, std::optional<EntityId>>, (size_t)Groups::size> entityGroups{};
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