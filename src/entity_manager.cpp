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

		if (entityGroups.count(e.hasComponentsBitmask) == 0) {
			entityGroups.emplace(e.hasComponentsBitmask, std::vector<EntityId>{entityCounter});
		}
		else {
			entityGroups.at(e.hasComponentsBitmask).push_back(entityCounter);
		}

		entityCounter += 1;

		constexpr EntityId components_size = std::tuple_size_v<RegisteredComponentsStorage>;
		constexpr EntityId inverter_mask = (1 << components_size) - 1; // 2 to the power of 8 (or currrent components_size) - 1 = all bits set to 1

		if (contigiousComponentsBlocks.size() == 0) {
			// example: 00010100 (used components) ^ 11111111 (xor inverter) -> components that weren't used
			EntityId unused_components_mask = e.hasComponentsBitmask ^ inverter_mask;
			contigiousComponentsBlocks.push_back(ContigiuousComponentsBlock{ 1, e.hasComponentsBitmask,  unused_components_mask });
		}
		else if (contigiousComponentsBlocks.back().hasComponentsBitmask != e.hasComponentsBitmask) {
			EntityId unused_components_mask = e.hasComponentsBitmask ^ inverter_mask;
			contigiousComponentsBlocks.push_back(ContigiuousComponentsBlock{ 1, e.hasComponentsBitmask,  unused_components_mask, contigiousComponentsBlocks.back().next_offsets});
		}

		auto& current_block = contigiousComponentsBlocks.back();
		current_block.entityCount += 1; // unused, faulty at this place and maybe use for offset (so just calculated once)
		// TODO: refactor to use entityCount once

		for (EntityId component_index = 0; component_index < components_size; component_index++) {

			// example: 00010100 (unused components) & 00000100 (and component index) > 0 -> component index was not used, add offset
			if ((current_block.unusedComponentsBitMask & (0 | 1 << component_index)) > 0) {
				current_block.next_offsets[component_index] += 1;
			}
				
		}

		e.blockId = contigiousComponentsBlocks.size() - 1;
		entities.push_back(e);
		commitStage = false;
	}

	void Manager::lock() {
		assert(!commitStage && "Tried to lock ecs manager but uncommited entity currently exists.");
		locked = true;
	}

	void Manager::reserve_size_entities(size_t size) {
		entities.reserve(size);
	}

}