#pragma once

#include "camera.hpp"
#include "game_object.hpp"

// lib
#include "vulkan/vulkan.h";

namespace SJFGame::Engine {

	const int MAX_LIGHTS{ 10 };

	struct PointLight {
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

	// check alignment rules before making changes to buffer object structures
	// https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/app09lev1sec2.html
	// A UBO is constant data available across all shaders set per frame
	struct GlobalUniformBufferOutput {
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		// vec3 and vec4 needs to be aligned to 16 Bytes
		// Options: 
		// 1. Place a padding member inbetween (tight packing by the host, cpu/gpu layout matching)
		// vec3 A;
		// uint32_t padding;
		// vec4 B;
		// 2. Instead of vec3 use a vec4 ignoring the w component 
		// (avoiding any bugs in the external implementation; 
		// Should not be a issue with Vulkan because Shaders are compiled to SPIR-V)
		// 3. Use alignas(16) to fill up space between vec3 and vec4
		glm::vec3 directionalLightPosition = { 4.f, -3.f, -1.f };
		alignas(16) glm::vec4 directionalLightColor = { 0.f, 1.0f, .3f, 0.f }; // w is intensity
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .05f }; // w is intensity

		PointLight pointLights[MAX_LIGHTS];
		int numLights{};
	};

	struct Frame {
		int frameIndex;
		float delta;
		Camera& camera;
		VkCommandBuffer cmdBuffer;
		VkDescriptorSet globalDescriptorSet;
		GameObject::Map& gameObjects;
	};
}
