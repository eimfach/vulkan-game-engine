#include "main_render_system.hpp"

#include "frame.hpp"

namespace nEngine::Engine {
	MainRenderSystem::MainRenderSystem(Device& device) : device{device} {

		globalPool = Engine::DescriptorPool::Builder(device)
			.setMaxSets(Engine::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Engine::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Engine::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		for (size_t i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_shared<Buffer>(
				device,
				sizeof(GlobalUniformBufferOutput),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::shared_ptr<Texture> texture = std::make_shared<Texture>( device, "textures/missing.png" );

		VkDescriptorImageInfo image_info{};
		image_info.sampler = texture->getSampler();
		image_info.imageView = texture->getImageView();
		image_info.imageLayout = texture->getImageLayout();

		texturePool.push_back({ texture, image_info });

		for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
			auto buffer_info = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &buffer_info)
				.writeImage(1, &texturePool[0].descriptor_info)
				.build(globalDescriptorSets[i]);
		}
	}

	MainRenderSystem::~MainRenderSystem() {
		globalPool = nullptr;
	}
}