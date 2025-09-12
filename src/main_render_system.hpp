#pragma once

#include "descriptors.hpp"
#include "swap_chain.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "texture.hpp"

#include <memory>
#include <vector>
#include <utility>

namespace nEngine::Engine {

	struct TexturePoolItem {
		std::shared_ptr<Texture> texture{};
		VkDescriptorImageInfo descriptor_info{};
	};

	class MainRenderSystem {
	public:
		MainRenderSystem(Device& device);
		~MainRenderSystem();

		// delete copy constructor and copy operator
		MainRenderSystem(const MainRenderSystem&) = delete;
		MainRenderSystem& operator= (const MainRenderSystem&) = delete;

		VkDescriptorSetLayout getGobalSetLayout() const { return globalSetLayout->getDescriptorSetLayout(); }
		VkDescriptorSet getGlobalDiscriptorSet(int frame_index) { return globalDescriptorSets[frame_index]; }
		std::shared_ptr<Buffer> getUboBuffer(int frame_index) { return uboBuffers[frame_index]; }

	private:
		Device& device;
		std::unique_ptr<DescriptorPool> globalPool{};
		std::unique_ptr<DescriptorSetLayout> globalSetLayout{};
		std::vector<std::shared_ptr<Buffer>> uboBuffers{ SwapChain::MAX_FRAMES_IN_FLIGHT };
		std::vector<VkDescriptorSet> globalDescriptorSets{ SwapChain::MAX_FRAMES_IN_FLIGHT };

		std::vector<TexturePoolItem> texturePool{};
	};
}