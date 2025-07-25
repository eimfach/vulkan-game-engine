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

		constexpr EntityId components_size = std::tuple_size_v<GeneralComponentStorage>;
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

	void Manager::reserve_size_entities(size_t size) {
		entities.reserve(size);
	}

	glm::mat4& Transform::mat4() {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		transformMatrixCache = glm::mat4{
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

		return transformMatrixCache;
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

	AABB::AABB(const std::vector<Engine::VertexBase>& verticies, Engine::Device& device) {
		for (auto& vertex : verticies) {
			auto position = vertex.position;
			if (position.z > min.z) {
				min.z = position.z;
			}
			if (position.y > min.y) {
				min.y = position.y;
			}
			if (position.x < min.x) {
				min.x = position.x;
			}

			if (position.z < max.z) {
				max.z = position.z;
			}
			if (position.y < max.y) {
				max.y = position.y;
			}
			if (position.x > max.x) {
				max.x = position.x;
			}

		}

		Engine::VertexModel::Builder model_builder{};

		model_builder.indicies = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
				12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		model_builder.verticies = {
			// Vertices defined by mapping the original cube's coordinates:
			// -0.5f -> min component
			// +0.5f -> max component

			// Left face (white, x = min.x)
			{{min.x, min.y, min.z}, {.9f, .9f, .9f}}, // 0
			{{min.x, max.y, max.z}, {.9f, .9f, .9f}}, // 1
			{{min.x, min.y, max.z}, {.9f, .9f, .9f}}, // 2
			{{min.x, max.y, min.z}, {.9f, .9f, .9f}}, // 3

			// Right face (yellow, x = max.x)
			{{max.x, min.y, min.z}, {.8f, .8f, .1f}}, // 4
			{{max.x, max.y, max.z}, {.8f, .8f, .1f}}, // 5
			{{max.x, min.y, max.z}, {.8f, .8f, .1f}}, // 6
			{{max.x, max.y, min.z}, {.8f, .8f, .1f}}, // 7

			// Top face (orange, y = min.y)
			{{min.x, min.y, min.z}, {.9f, .6f, .1f}}, // 8
			{{max.x, min.y, max.z}, {.9f, .6f, .1f}}, // 9
			{{min.x, min.y, max.z}, {.9f, .6f, .1f}}, // 10
			{{max.x, min.y, min.z}, {.9f, .6f, .1f}}, // 11

			// Bottom face (red, y = max.y)
			{{min.x, max.y, min.z}, {.8f, .1f, .1f}}, // 12
			{{max.x, max.y, max.z}, {.8f, .1f, .1f}}, // 13
			{{min.x, max.y, max.z}, {.8f, .1f, .1f}}, // 14
			{{max.x, max.y, min.z}, {.8f, .1f, .1f}}, // 15

			// Nose face (blue, z = max.z)
			{{min.x, min.y, max.z}, {.1f, .1f, .8f}}, // 16
			{{max.x, max.y, max.z}, {.1f, .1f, .8f}}, // 17
			{{min.x, max.y, max.z}, {.1f, .1f, .8f}}, // 18
			{{max.x, min.y, max.z}, {.1f, .1f, .8f}}, // 19

			// Tail face (green, z = min.z)
			{{min.x, min.y, min.z}, {.1f, .8f, .1f}}, // 20
			{{max.x, max.y, min.z}, {.1f, .8f, .1f}}, // 21
			{{min.x, max.y, min.z}, {.1f, .8f, .1f}}, // 22
			{{max.x, min.y, min.z}, {.1f, .8f, .1f}}, // 23
		};

		//for (auto& v : model_builder.verticies) {
		//	v.position += offset;
		//}
		debugModel = std::make_shared<Engine::VertexModel>(device, model_builder);
	}

}