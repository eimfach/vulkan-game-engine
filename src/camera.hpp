#pragma once

#include "aabb.hpp"

// libs
// don't use degrees, force use radians
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <array>

namespace SJFGame::Engine {
	const glm::vec3 UP{ 0.f, -1.f, 0.f };
	const glm::vec3 RIGHT{ 1.f, 0.f, 0.f };
	const glm::vec3 FRONT{ 0.f, 0.f, -1.f };

	using Frustum = std::array<glm::vec4, 6>;

	class Camera {
	public:
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fov_y, float aspect, float near, float far);
		
		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = UP);
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = UP);
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& getProjection() const { return projectionMatrix; }	
		const glm::mat4& getView() const { return viewMatrix; }
		const glm::mat4& getInverseView() const { return inverseViewMatrix; }
		const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); }

		const Frustum produceFrustum();
		bool isWorldSpaceAABBfrustumVisible(const SJFGame::ECS::AABB& aabb) const;

	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{ 1.f };
		Frustum frustum{};

		bool isWorldSpaceAABBinsidePlane(const glm::vec3& center, const glm::vec3& size, const glm::vec4& plane) const;
	};
}