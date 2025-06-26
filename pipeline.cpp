#include "pipeline.hpp"

#include <fstream>
#include <iostream>
#include <cassert>

namespace bm {
	Pipeline::Pipeline(Device& device, const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath) : device{device} {
		createGraphicsPipeline(cfg, vertex_filepath, fragment_filepath);
	}

	Pipeline::~Pipeline() {
		vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);
		vkDestroyShaderModule(device.device(), fragmentShaderModule, nullptr);
		vkDestroyPipeline(device.device(), gpuPipeline, nullptr);
	}

	std::vector<char> Pipeline::read_file(const std::string& filepath) {
		std::ifstream file{ filepath, std::ios::ate, std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filepath);
		}

		const size_t file_size = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(file_size);
		file.seekg(0);
		file.read(buffer.data(), static_cast<std::streamsize>(file_size));

		file.close();
		return buffer;
	}

	void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = code.size();
		info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(device.device(), &info, nullptr, shaderModule) != VK_SUCCESS) {
			std::runtime_error("failed to create shader module");
		}
	}

	void Pipeline::createGraphicsPipeline(const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath) {
		assert(cfg.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided in config");
		assert(cfg.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided in config");

		auto vertex_code = read_file(vertex_filepath);
		auto fragment_code = read_file(fragment_filepath);

		std::cout << "Vertex Shader Code size is: " << vertex_code.size() << "\n";
		std::cout << "Fragment Shader Code size is: " << fragment_code.size() << "\n";

		createShaderModule(vertex_code, &vertexShaderModule);
		createShaderModule(fragment_code, &fragmentShaderModule);

		VkPipelineShaderStageCreateInfo shader_stages[2];
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = vertexShaderModule;
		shader_stages[0].pName = "main";
		shader_stages[0].flags = 0;
		shader_stages[0].pNext = nullptr;
		shader_stages[0].pSpecializationInfo = nullptr;

		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = fragmentShaderModule;
		shader_stages[1].pName = "main";
		shader_stages[1].flags = 0;
		shader_stages[1].pNext = nullptr;
		shader_stages[1].pSpecializationInfo = nullptr;

		VkPipelineVertexInputStateCreateInfo vertex_input{};
		vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input.vertexAttributeDescriptionCount = 0;
		vertex_input.vertexBindingDescriptionCount = 0;
		vertex_input.pVertexAttributeDescriptions = nullptr;
		vertex_input.pVertexBindingDescriptions = nullptr;

		VkPipelineViewportStateCreateInfo viewport{};
		// combine viewport and scissor
		viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport.viewportCount = 1;
		viewport.pViewports = &cfg.viewport;
		viewport.scissorCount = 1;
		viewport.pScissors = &cfg.scissor;

		VkGraphicsPipelineCreateInfo pipeline{};
		pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline.stageCount = 2;
		pipeline.pStages = shader_stages;
		pipeline.pVertexInputState = &vertex_input;
		pipeline.pInputAssemblyState = &cfg.inputAssemblyInfo;
		pipeline.pViewportState = &viewport;
		pipeline.pRasterizationState = &cfg.rasterizationInfo;
		pipeline.pMultisampleState = &cfg.multisampleInfo;
		pipeline.pColorBlendState = &cfg.colorBlendInfo;
		pipeline.pDepthStencilState = &cfg.depthStencilInfo;
		pipeline.pDynamicState = nullptr;

		pipeline.layout = cfg.pipelineLayout;
		pipeline.renderPass = cfg.renderPass;
		pipeline.subpass = cfg.subpass;

		pipeline.basePipelineIndex = -1;
		pipeline.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipeline, nullptr, &gpuPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create gpu pipeline");
		}

	}

	PipelineConfig Pipeline::defaultCfg(uint32_t width, uint32_t height) {
		PipelineConfig cfg{};

		// INPUT ASSEMBLY STAGE

		cfg.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		cfg.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		cfg.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// how to transistion our gl_Position (from the vertex shader code) values to the output images
		cfg.viewport.x = 0.0f;
		cfg.viewport.y = 0.0f;
		cfg.viewport.width = static_cast<float>(width);
		cfg.viewport.height = static_cast<float>(height);
		// z-component of gl_Position:
		cfg.viewport.minDepth = 0.0f;
		cfg.viewport.maxDepth = 1.0f;

		// discard/cut pixels
		cfg.scissor.offset = { 0, 0 };
		cfg.scissor.extent = { width, height };

		// RASTERIZATION STAGE

		cfg.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		cfg.rasterizationInfo.depthClampEnable = VK_FALSE;
		cfg.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		cfg.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		cfg.rasterizationInfo.lineWidth = 1.0f;
		cfg.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		cfg.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		cfg.rasterizationInfo.depthBiasEnable = VK_FALSE;
		cfg.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
		cfg.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
		cfg.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

		cfg.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		cfg.multisampleInfo.sampleShadingEnable = VK_FALSE;
		cfg.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		cfg.multisampleInfo.minSampleShading = 1.0f;           // Optional
		cfg.multisampleInfo.pSampleMask = nullptr;             // Optional
		cfg.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		cfg.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		cfg.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		cfg.colorBlendAttachment.blendEnable = VK_FALSE;
		cfg.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		cfg.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		cfg.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		cfg.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		cfg.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		cfg.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		cfg.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cfg.colorBlendInfo.logicOpEnable = VK_FALSE;
		cfg.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		cfg.colorBlendInfo.attachmentCount = 1;
		cfg.colorBlendInfo.pAttachments = &cfg.colorBlendAttachment;
		cfg.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		cfg.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		cfg.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		cfg.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		cfg.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		cfg.depthStencilInfo.depthTestEnable = VK_TRUE;
		cfg.depthStencilInfo.depthWriteEnable = VK_TRUE;
		cfg.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		cfg.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		cfg.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
		cfg.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		cfg.depthStencilInfo.stencilTestEnable = VK_FALSE;
		cfg.depthStencilInfo.front = {};  // Optional
		cfg.depthStencilInfo.back = {};   // Optional

		return cfg;
	}
	
}