#pragma once

#include "device.hpp"

#include <string>
#include <vector>

namespace bm {
	struct PipelineConfig {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline {
	public:
		Pipeline(Device& device, const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath);
		~Pipeline();
		Pipeline(const Pipeline&) = delete;
		void operator= (const Pipeline&) = delete;

		static PipelineConfig defaultCfg(uint32_t width, uint32_t height);
		void bind(VkCommandBuffer command_buffer);

	private:
		Device& device;
		VkPipeline gpuPipeline;
		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		static std::vector<char> read_file(const std::string& filepath);
		void createGraphicsPipeline(const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	};
}