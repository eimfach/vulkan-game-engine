#pragma once

#include "device.hpp"

#include <string>
#include <vector>

namespace SJFGame::Engine {
	struct PipelineConfig {
		PipelineConfig() = default;
		PipelineConfig(const PipelineConfig&) = delete;
		PipelineConfig& operator=(const PipelineConfig&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;

		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		std::vector<VkDynamicState> dynamicStateEnables;

		uint32_t subpass = 0;
	};

	class Pipeline {
	public:
		Pipeline(Device& device, const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		static void defaultCfg(PipelineConfig& cfg);
		static void enableAlphaBlending(PipelineConfig& cfg);
		static void setTopology(PipelineConfig& cfg, VkPrimitiveTopology topology, VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL);
		void bind(VkCommandBuffer command_buffer);

	private:
		Device& device;
		VkPipeline gpuPipeline;
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		void createGraphicsPipeline(const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	};
}