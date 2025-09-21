#include "entity_manager.hpp"

namespace nEngine::ECS {
	Manager::Manager() {};
	Manager::~Manager() {};

	std::pair<Entity, EntityId> Manager::createEntity(bool set_default_components) {
		assert(!commitStage && "Cannot create new entity, because another is uncommited.");
		assert(!locked && "Cannot create new entity, because manager is locked.");

		commitStage = true;
		assert(entityCounter <= MAX_ENTITIES && "MAX Entities reached!");

		return { Entity{}, entityCounter };
	}

	void Manager::commit(Entity e) {
		assert(commitStage && "Cannot commit an entity, because one needs to be created first.");
		// 4a. entityGroups<std::vector<T>> thread unsafe
		// 4c. entityGroups.at(enum).pushBuffer(entityCounter);
		if (entityGroups.count(e.hasComponentsBitmask) == 0) {
			entityGroups.emplace(e.hasComponentsBitmask, std::vector<EntityId>{entityCounter});
		}
		else {
			entityGroups.at(e.hasComponentsBitmask).push_back(entityCounter);
		}

		constexpr EntityId components_size = std::tuple_size_v<RegisteredComponentsStorage>;
		constexpr EntityId inverter_mask = (1 << components_size) - 1; // 2 to the power of 8 (or currrent components_size) - 1 = all bits set to 1

		// 5. contigiousComponentsBlocks thread unsafe
		// Create a block if the prev block bitmask is unequal if not ...  
		// scope the ref
		// Make the changes to the created block (count + 1, setting offsets)
		// size(), back() is safe until you write to it

		// ContigiuousComponentsBlock current_block{};

		if (contigiousComponentsBlocks.size() == 0) {
			// example: 00010100 (used components) ^ 11111111 (xor inverter) -> components that weren't used
			EntityId unused_components_mask = e.hasComponentsBitmask ^ inverter_mask;
			// 5. current_block = ContigiuousComponentsBlock{ 0, e.hasComponentsBitmask,  unused_components_mask }
			contigiousComponentsBlocks.push_back(ContigiuousComponentsBlock{ 0, e.hasComponentsBitmask,  unused_components_mask });
		}
		else if (contigiousComponentsBlocks.back().hasComponentsBitmask != e.hasComponentsBitmask) {
			EntityId unused_components_mask = e.hasComponentsBitmask ^ inverter_mask;
			auto prev_offsets = contigiousComponentsBlocks.back().next_offsets;
			// 5. current_block = ContigiuousComponentsBlock{ 0, e.hasComponentsBitmask,  unused_components_mask, prev_offsets, prev_offsets}
			contigiousComponentsBlocks.push_back(ContigiuousComponentsBlock{ 0, e.hasComponentsBitmask,  unused_components_mask, prev_offsets, prev_offsets});
		}
		// else {
		//    current_block.merge = true;
		// }

		// 5a. Unsafe
		auto& current_block = contigiousComponentsBlocks.back();
		current_block.entityCount += 1; // TODO: unused
		// TODO: refactor to use entityCount once

		for (EntityId component_index = 0; component_index < components_size; component_index++) {

			// example: 00010100 (unused components) & 00000100 (and component index) > 0 -> component index was not used, add offset
			if ((current_block.unusedComponentsBitMask & (1 << component_index)) > 0) {
				current_block.next_offsets[component_index] += 1;
			}

		} // 5.a

		// 5. contigiousComponentsBlocks.pushBuffer(current_block);
		entityCounter += 1;
		e.blockId = contigiousComponentsBlocks.size() - 1; // 5. should be size because it will be + 1 in the vector after merging
		entities.push_back(e);
		commitStage = false;
	}

	void Manager::lock() {
		assert(!commitStage && "Tried to lock ecs manager but uncommited entity currently exists.");
		locked = true;
	}

	void Manager::reserveSizeEntities(size_t size) {
		entities.reserve(size);
	}

}