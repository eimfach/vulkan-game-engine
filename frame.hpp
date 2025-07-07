#pragma once

#include "camera.hpp"

// lib
#include "vulkan/vulkan.h";

namespace Biosim::Engine {
	struct Frame {
		int frameIndex;
		float delta;
		Camera& camera;
		VkCommandBuffer cmdBuffer;
		VkDescriptorSet globalDescriptorSet;
	};
}
