#pragma once

#include "device.hpp"

//libs
#include <vulkan/vulkan.h>

// std
#include <string>

namespace nEngine::Engine {
	class Texture {
	public:
		Texture(Device& device, const std::string& filepath);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) = delete;
		Texture& operator=(Texture&&) = delete;

		VkSampler getSampler() { return sampler; }
		VkImageView getImageView() { return imageView; }
		VkImageLayout getImageLayout() { return imageLayout; }

	private:
		Device& device;
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
		VkSampler sampler;
		VkFormat imageFormat;
		VkImageLayout imageLayout;
		int width;
		int height;
		int mipLevels;

		void transitionImageLayout(VkImageLayout old_layout, VkImageLayout new_layout);
		void generateMipmaps();
	};
}