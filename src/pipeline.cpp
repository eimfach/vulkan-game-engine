#include "pipeline.hpp"

#include "vertex_model.hpp"
#include "utils.hpp"

#include <fstream>
#include <cassert>

// TODO: need better solution
#ifndef ENGINE_DIR
#define ENGINE_DIR "./"
#endif

namespace SJFGame::Engine {
	Pipeline::Pipeline(Device& device, const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath) : device{device} {
		createGraphicsPipeline(cfg, vertex_filepath, fragment_filepath);
	}

	Pipeline::~Pipeline() {
		vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);
		vkDestroyShaderModule(device.device(), fragmentShaderModule, nullptr);
		vkDestroyPipeline(device.device(), gpuPipeline, nullptr);
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

	void Pipeline::bind(VkCommandBuffer command_buffer) {
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpuPipeline);

	}

	void Pipeline::createGraphicsPipeline(const PipelineConfig& cfg, const std::string& vertex_filepath, const std::string& fragment_filepath) {
		assert(cfg.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided in config");
		assert(cfg.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipeline layout provided in config");

		auto vertex_code = Utils::read_file(vertex_filepath);
		auto fragment_code = Utils::read_file(fragment_filepath);

		//std::cout << "Vertex Shader Code size is: " << vertex_code.size() << "\n";
		//std::cout << "Fragment Shader Code size is: " << fragment_code.size() << "\n";

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

		auto& vertex_bindings = cfg.bindingDescriptions;
		auto& vertex_attributes = cfg.attributeDescriptions;

		VkPipelineVertexInputStateCreateInfo vertex_input{};
		vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes.size());
		vertex_input.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_bindings.size());;
		vertex_input.pVertexAttributeDescriptions = vertex_attributes.data();
		vertex_input.pVertexBindingDescriptions = vertex_bindings.data();

		VkPipelineColorBlendStateCreateInfo color_blend_info{};
		color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_info.logicOpEnable = VK_FALSE;
		color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
		color_blend_info.attachmentCount = 1;
		color_blend_info.pAttachments = &cfg.colorBlendAttachment;
		color_blend_info.blendConstants[0] = 0.0f;  // Optional
		color_blend_info.blendConstants[1] = 0.0f;  // Optional
		color_blend_info.blendConstants[2] = 0.0f;  // Optional
		color_blend_info.blendConstants[3] = 0.0f;  // Optional

		VkGraphicsPipelineCreateInfo pipeline{};
		pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline.stageCount = 2;
		pipeline.pStages = shader_stages;
		pipeline.pVertexInputState = &vertex_input;
		pipeline.pInputAssemblyState = &cfg.inputAssemblyInfo;
		pipeline.pViewportState = &cfg.viewportInfo;
		pipeline.pRasterizationState = &cfg.rasterizationInfo;
		pipeline.pMultisampleState = &cfg.multisampleInfo;
		pipeline.pColorBlendState = &color_blend_info;
		pipeline.pDepthStencilState = &cfg.depthStencilInfo;
		pipeline.pDynamicState = &cfg.dynamicStateInfo;

		pipeline.layout = cfg.pipelineLayout;
		pipeline.renderPass = cfg.renderPass;
		pipeline.subpass = cfg.subpass;

		pipeline.basePipelineIndex = -1;
		pipeline.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipeline, nullptr, &gpuPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create gpu pipeline");
		}

	}

	void Pipeline::defaultCfg(PipelineConfig& cfg) {
		// INPUT ASSEMBLY STAGE
		cfg.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		cfg.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Relevant
		cfg.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// combine viewport and scissor
		cfg.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		cfg.viewportInfo.viewportCount = 1;
		cfg.viewportInfo.pViewports = nullptr;
		cfg.viewportInfo.scissorCount = 1;
		cfg.viewportInfo.pScissors = nullptr;

		// RASTERIZATION STAGE

		cfg.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		cfg.rasterizationInfo.depthClampEnable = VK_FALSE;
		cfg.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		cfg.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL; // Relevant
		cfg.rasterizationInfo.lineWidth = 1.0f;
		cfg.rasterizationInfo.cullMode = VK_CULL_MODE_NONE; // Relevant
		cfg.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // Relevant
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

		cfg.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		cfg.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		cfg.dynamicStateInfo.pDynamicStates = cfg.dynamicStateEnables.data();
		cfg.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(cfg.dynamicStateEnables.size());
		cfg.dynamicStateInfo.flags = 0;

		cfg.bindingDescriptions = VertexModel::Vertex::getBindingDescriptions();
		cfg.attributeDescriptions = VertexModel::Vertex::getAttributeDescriptions();
	}

	void Pipeline::enableAlphaBlending(PipelineConfig& cfg) {
		cfg.colorBlendAttachment.blendEnable = VK_TRUE;

		cfg.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		cfg.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		cfg.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		cfg.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		cfg.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   
		cfg.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		cfg.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void Pipeline::setTopology(PipelineConfig& cfg, VkPrimitiveTopology topology, VkPolygonMode polygon_mode) {
		cfg.inputAssemblyInfo.topology = topology;
		cfg.rasterizationInfo.polygonMode = polygon_mode;
	}
	
}