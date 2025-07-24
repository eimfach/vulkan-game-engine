#include "entity_manager.hpp"

namespace SJFGame::ECS {
	Manager::Manager() {};
	Manager::~Manager() {};

	Entity Manager::createEntity(bool set_default_components) {
		assert(!commitStage && "Cannot create new entity, because another is uncommited");
		commitStage = true;
		assert(counter < MAX_ENTITIES && "MAX Entities reached!");

		Entity e{ counter++, {} };
		return e;
	}

	void Manager::commit(Entity e) {
		if (entityGroups.count(e.hasComponentsBitmask) == 0) {
			entityGroups.emplace(e.hasComponentsBitmask, std::vector<EntityId>{e.id});
		}
		else {
			entityGroups.at(e.hasComponentsBitmask).push_back(e.id);
		}
		entities.push_back(e);
		commitStage = false;
	}

	glm::mat4 Transform::mat4() const {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{
				translation.x,
				translation.y,
				translation.z,
				1.0f
			}
		};
	}

	glm::mat3 Transform::normalMatrix() const {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 inv_scale = 1.0f / scale;

		return glm::mat3{
			{
				inv_scale.x * (c1 * c3 + s1 * s2 * s3),
				inv_scale.x * (c2 * s3),
				inv_scale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				inv_scale.y * (c3 * s1 * s2 - c1 * s3),
				inv_scale.y * (c2 * c3),
				inv_scale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				inv_scale.z * (c2 * s1),
				inv_scale.z * (-s2),
				inv_scale.z * (c1 * c2),
			},
		};
	}

	AABB::AABB(std::vector<glm::vec3> verticies) {
		for (auto& vec : verticies) {
			if (vec.z > min.z && vec.y > min.y) {
				min = vec;
			}
			else if (vec.z < max.z && vec.y < max.y) {
				max = vec;
			}
		}
	}
}