#include "entity_manager.hpp"

namespace SJFGame::ECS {
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

	glm::mat4 Transform::modelMatrix() {
		glm::mat4 transformMatrix{1.f};
		transformMatrix = glm::translate(transformMatrix, translation);
		transformMatrix = glm::rotate(transformMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		transformMatrix = glm::rotate(transformMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		transformMatrix = glm::scale(transformMatrix, scale);
		return transformMatrix;

		//const float c3 = glm::cos(rotation.z);
		//const float s3 = glm::sin(rotation.z);
		//const float c2 = glm::cos(rotation.x);
		//const float s2 = glm::sin(rotation.x);
		//const float c1 = glm::cos(rotation.y);
		//const float s1 = glm::sin(rotation.y);
		//transformMatrixCache = glm::mat4{
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
		//return transformMatrixCache;
	}

	glm::mat3 Transform::normalMatrix(glm::mat4& modelMatrix) {
		return glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
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
}