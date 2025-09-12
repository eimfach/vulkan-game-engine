#include "movement.hpp"

namespace nEngine::Engine {
	void MovementControl::moveInPlaneXZ(GLFWwindow* window, float delta, ECS::Transform& transform) {
		glm::vec3 rotate{};

		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) { rotate.y += 1.f; }
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) { rotate.y -= 1.f; }
		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) { rotate.x += 1.f; }
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) { rotate.x -= 1.f; }

		
		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			// scale rotate with lookspeed and delta time (frame time unindepentent) 
			// and normalize the rotation vector to be even in speed in all directions (diagonal is faster)
			transform.rotation += lookSpeed * delta * glm::normalize(rotate);
		}

		// clamp angle ranges
		transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
		transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());
		
		// face direction
		float yaw = transform.rotation.y;
		const glm::vec3 forward_dir{ glm::sin(yaw), 0.f, glm::cos(yaw) };
		const glm::vec3 right_dir{ forward_dir.z, 0.f, -forward_dir.x };
		const glm::vec3 up_dir{ 0.f, -1.f, 0.f };

		glm::vec3 move_dir{ 0.f };
		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) { move_dir += forward_dir; }
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) { move_dir -= forward_dir; }
		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) { move_dir += right_dir; }
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) { move_dir -= right_dir; }
		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) { move_dir += up_dir; }
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) { move_dir -= up_dir; }

		if (glm::dot(move_dir, move_dir) > std::numeric_limits<float>::epsilon()) {
			// scale rotate with lookspeed and delta time (frame time unindepentent) 
			// and normalize the rotation vector to be even in speed in all directions (diagonal is faster)
			transform.translation += moveSpeed * delta * glm::normalize(move_dir);
		}
	}
}