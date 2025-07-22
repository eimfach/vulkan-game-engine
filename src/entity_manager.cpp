#include "entity_manager.hpp"

namespace SJFGame::ECS {
	Manager::Manager() {};
	Manager::~Manager() {};

	Entity Manager::createEntity() {
		assert(!commitStage && "Cannot create new entity, because another is uncommited");
		commitStage = true;
		assert(counter < MAX_ENTITIES && "MAX Entities reached!");

		return { ++counter, {} };
	}

	void Manager::commit(Entity e) {
		if (!entityGroups.contains(e.has_components_in)) {
			entityGroups.emplace(e.has_components_in, e);
		}
		else {
			entityGroups.at(e.has_components_in).push_back(e.id);
		}
		commitStage = false;
	}

	std::vector<EntityId>& Manager::getGroup(ComponentsMask mask) {
		assert(entityGroups.contains(mask) && "Requested Component Group does not exist!");
		return entityGroups.at(mask);
	}
}