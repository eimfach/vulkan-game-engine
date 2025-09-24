#pragma once

#include "buffered_vector.hpp"
#include "vertex_model.hpp"
#include "aabb.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// std
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace nEngine::ECS {

	using EntityId = std::uint16_t;
	using ComponentsMask = std::uint16_t;

	struct Entity {
		ComponentsMask hasComponentsBitmask{};
		EntityId blockId{};
	};

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
		bool visible{ true };
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
		}

		inline glm::mat3 normalMatrix(glm::mat4& model_matrix) {
			return glm::transpose(glm::inverse(glm::mat3(model_matrix)));
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


	using RegisteredComponentsStorage = std::tuple<
		BufferedVector<Identification>,
		BufferedVector<Visibility>,
		BufferedVector<Transform>,
		BufferedVector<PointLight>,
		BufferedVector<Mesh>,
		BufferedVector<Color>,
		BufferedVector<RenderLines>,
		BufferedVector<AABB>
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
	struct ContiguousComponentsBlock {
		EntityId entityCount{};
		ComponentsMask hasComponentsBitmask{};
		ComponentsMask unusedComponentsBitMask{};
		std::array<EntityId, std::tuple_size_v<RegisteredComponentsStorage>> offsets{}; // no offsets can be changed at the moment (copy), after the block was connected with another block 
		std::array<EntityId, std::tuple_size_v<RegisteredComponentsStorage>> next_offsets{};
		bool merge{ false };

		ContiguousComponentsBlock& operator+=(ContiguousComponentsBlock& rhs) {
			entityCount += 1;
			for (int i{}; i < next_offsets.size(); i++) {
				if (rhs.next_offsets[i] == 0) {
					continue;
				}
				next_offsets[i] += 1;
			}

			return *this;
		}
	};
}

namespace nEngine {
	template<>
	inline bool BufferedVector<ECS::ContiguousComponentsBlock>::syncElement() {
		std::lock_guard<std::mutex> lock{ mutex };

		if (!buffer.empty()) {
			if (buffer.front().merge) {
				_merge();
				return true;
			}

			_sync();
			return true;
		}

		return false;
	}

	template<>
	inline bool BufferedVector<ECS::EntityId, std::optional<ECS::EntityId>>::syncElement() {
		std::lock_guard<std::mutex> lock{ mutex };

		if (!buffer.empty()) {
			if (!buffer.front()) {
				buffer.pop();
			}
			else {
				data.push_back(std::move(buffer.front().value()));
				buffer.pop();
			}

			return true;
		}

		return false;
	}
}