#pragma once

#include "camera.hpp"
#include "game_object.hpp"

// lib
#include "vulkan/vulkan.h";

namespace SJFGame::Engine {
	struct Frame {
		int frameIndex;
		float delta;
		Camera& camera;
		VkCommandBuffer cmdBuffer;
		VkDescriptorSet globalDescriptorSet;
		const GameObject::Map& gameObjects;
	};
}
