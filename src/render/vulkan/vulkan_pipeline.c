#include "stupid/render/vulkan/vulkan_pipeline.h"
#include "stupid/render/vulkan/vulkan_utils.h"

#include "stupid/memory.h"

#include <stdio.h>

static inline char *loadShaderFile(const char *path, usize *size)
{
	FILE *f = fopen(path, "r");
	STUPID_NC(f);

	fseek(f, 0, SEEK_END);
	usize fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buf = stMemAllocs(fsize);

	usize b = 0;

	//while ((b = fread(buf + b, 1, fsize - *size, f)) > 0)
	while ((b = fread(buf, 1, fsize, f)) > 0)
		*size += b;

	fclose(f);

	if (b < 0) {
		perror("fread");
		stMemDealloc(buf);
	}
	
	return buf;
}

static void stRendererVulkanPipelineSetupDynamicState(StRendererVulkanPipeline *pPipeline)
{
        stMemAppend(pPipeline->info.pDynamicStates, VK_DYNAMIC_STATE_VIEWPORT);
        stMemAppend(pPipeline->info.pDynamicStates, VK_DYNAMIC_STATE_SCISSOR);

        stMemset(&pPipeline->info.dynamic_state_info, 0, sizeof(pPipeline->info.dynamic_state_info));
        pPipeline->info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pPipeline->info.dynamic_state_info.dynamicStateCount = stMemLength(pPipeline->info.pDynamicStates);
        pPipeline->info.dynamic_state_info.pDynamicStates = pPipeline->info.pDynamicStates;
}

static void stRendererVulkanPipelineSetupVertexInputState(StRendererVulkanPipeline *pPipeline)
{
        stMemset(&pPipeline->info.vertex_input_info, 0, sizeof(pPipeline->info.vertex_input_info));
        pPipeline->info.vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
}

static void stRendererVulkanPipelineSetupInputAssemblyState(StRendererVulkanPipeline *pPipeline, VkPrimitiveTopology topology)
{
        stMemset(&pPipeline->info.input_assembly_info, 0, sizeof(pPipeline->info.input_assembly_info));
        pPipeline->info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pPipeline->info.input_assembly_info.topology = topology;
}

static void stRendererVulkanPipelineSetupRasterizationState(StRendererVulkanPipeline *pPipeline, VkPolygonMode mode)
{
        stMemset(&pPipeline->info.rasterization_info, 0, sizeof(pPipeline->info.rasterization_info));
        pPipeline->info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pPipeline->info.rasterization_info.polygonMode = mode;
        pPipeline->info.rasterization_info.lineWidth = 1.0f;
        pPipeline->info.rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT;
        pPipeline->info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pPipeline->info.rasterization_info.depthBiasConstantFactor = 0.0f;
        pPipeline->info.rasterization_info.depthBiasClamp = 0.0f;
        pPipeline->info.rasterization_info.depthBiasSlopeFactor = 0.0f;
}

static void stRendererVulkanPipelineSetupMultisampleState(StRendererVulkanPipeline *pPipeline, VkSampleCountFlagBits sample_count)
{
        stMemset(&pPipeline->info.multisample_info, 0, sizeof(pPipeline->info.multisample_info));
        pPipeline->info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pPipeline->info.multisample_info.rasterizationSamples = sample_count;
        pPipeline->info.multisample_info.minSampleShading = 1.0f;
}


static void stRendererVulkanPipelineSetupColorBlendAttachmentState(StRendererVulkanPipeline *pPipeline)
{
        stMemset(&pPipeline->info.color_blend_attachment_state, 0, sizeof(pPipeline->info.color_blend_attachment_state));
        pPipeline->info.color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                                      VK_COLOR_COMPONENT_G_BIT |
                                                                      VK_COLOR_COMPONENT_B_BIT |
                                                                      VK_COLOR_COMPONENT_A_BIT;
}

static void stRendererVulkanPipelineSetupViewportState(StRendererVulkanPipeline *pPipeline, VkViewport *pViewports, u32 viewport_count, VkRect2D *pScissors, u32 scissor_count)
{
        stMemset(&pPipeline->info.viewport_info, 0, sizeof(pPipeline->info.viewport_info));
        pPipeline->info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pPipeline->info.viewport_info.viewportCount = viewport_count;
        pPipeline->info.viewport_info.scissorCount = scissor_count;
        pPipeline->info.viewport_info.pViewports = pViewports;
        pPipeline->info.viewport_info.pScissors = pScissors;
}

static void stRendererVulkanPipelineSetupColorBlendState(StRendererVulkanPipeline *pPipeline)
{
        stMemset(&pPipeline->info.color_blend_state_info, 0, sizeof(pPipeline->info.color_blend_state_info));
        pPipeline->info.color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pPipeline->info.color_blend_state_info.logicOp = VK_LOGIC_OP_COPY;
        pPipeline->info.color_blend_state_info.attachmentCount = 1;
        pPipeline->info.color_blend_state_info.pAttachments = &pPipeline->info.color_blend_attachment_state;
}

static void stRendererVulkanPipelineSetupDepthStencilState(StRendererVulkanPipeline *pPipeline)
{
        stMemset(&pPipeline->info.depth_stencil_info, 0, sizeof(pPipeline->info.depth_stencil_info));
        pPipeline->info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pPipeline->info.depth_stencil_info.depthTestEnable = VK_TRUE;
        pPipeline->info.depth_stencil_info.depthWriteEnable = VK_TRUE;
        pPipeline->info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pPipeline->info.depth_stencil_info.stencilTestEnable = VK_FALSE;
}

static void stRendererVulkanPipelineSetupLayout(StRendererVulkanPipeline *pPipeline, VkPushConstantRange *pRange, const u32 range_count)
{
        stMemset(&pPipeline->info.layout_info, 0, sizeof(pPipeline->info.layout_info));
        pPipeline->info.layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipeline->info.layout_info.pPushConstantRanges = pRange;
	pPipeline->info.layout_info.pushConstantRangeCount = range_count;
}

static void stRendererVulkanPipelineSetupGraphicsPipeline(StRendererVulkanPipeline *pPipeline, VkPipelineShaderStageCreateInfo *pShaderInfo, const usize shader_count, VkFormat *pColorFormats, u32 color_format_count, VkFormat depth_format)
{

        stMemset(&pPipeline->info.rendering_info, 0, sizeof(pPipeline->info.rendering_info));
        pPipeline->info.rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pPipeline->info.rendering_info.colorAttachmentCount = color_format_count;
        pPipeline->info.rendering_info.pColorAttachmentFormats = pColorFormats;
        pPipeline->info.rendering_info.depthAttachmentFormat = depth_format;
        pPipeline->info.rendering_info.stencilAttachmentFormat = 0;

        stMemset(&pPipeline->info.graphics_info, 0, sizeof(pPipeline->info.graphics_info));
        pPipeline->info.graphics_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pPipeline->info.graphics_info.pNext = &pPipeline->info.rendering_info;
        pPipeline->info.graphics_info.pStages = pShaderInfo;
        pPipeline->info.graphics_info.stageCount = shader_count;
        pPipeline->info.graphics_info.pMultisampleState = &pPipeline->info.multisample_info;
        pPipeline->info.graphics_info.pInputAssemblyState = &pPipeline->info.input_assembly_info;
        pPipeline->info.graphics_info.pVertexInputState = &pPipeline->info.vertex_input_info;
        pPipeline->info.graphics_info.pViewportState = &pPipeline->info.viewport_info;
        pPipeline->info.graphics_info.pRasterizationState = &pPipeline->info.rasterization_info;
        pPipeline->info.graphics_info.pColorBlendState = &pPipeline->info.color_blend_state_info;
        pPipeline->info.graphics_info.pDynamicState = &pPipeline->info.dynamic_state_info;
        pPipeline->info.graphics_info.pTessellationState = &pPipeline->info.tessellation_info;
        pPipeline->info.graphics_info.pDepthStencilState = &pPipeline->info.depth_stencil_info;
        pPipeline->info.graphics_info.layout = pPipeline->layout;
        pPipeline->info.graphics_info.basePipelineIndex = -1;
}

void stRendererVulkanPipelineCreate(const StRendererVulkanBackend *pBackend,
                                    StRendererVulkanPipeline *pPipeline,
                                    VkViewport *pViewports,
                                    const u32 num_viewports,
                                    VkRect2D *pScissors,
                                    const u32 num_scissors,
                                    VkFormat *pColorFormats,
                                    const u32 color_format_count,
                                    const VkFormat depth_format,
			            const char **shader_paths,
			            const VkShaderStageFlagBits *shader_stages,
			            const u32 shader_count,
				    VkPushConstantRange *pRange,
				    const u32 range_count)
{
        STUPID_NC(pBackend);
        STUPID_NC(pPipeline);

        stMemset(pPipeline, 0, sizeof(StRendererVulkanPipeline));

	VkPipelineShaderStageCreateInfo *pShaderStageInfo = stMemAlloc(VkPipelineShaderStageCreateInfo, shader_count);

        pPipeline->info.pDynamicStates = stMemAlloc(VkDynamicState, 2);
        pPipeline->info.pShaderData = stMemAlloc(void *, shader_count);
        stRendererVulkanPipelineSetupDynamicState(pPipeline);

        stRendererVulkanPipelineSetupMultisampleState(pPipeline, VK_SAMPLE_COUNT_1_BIT);
        stRendererVulkanPipelineSetupViewportState(pPipeline, pViewports, num_viewports, pScissors, num_scissors);
        stRendererVulkanPipelineSetupColorBlendAttachmentState(pPipeline);
        stRendererVulkanPipelineSetupColorBlendState(pPipeline);
        stRendererVulkanPipelineSetupDepthStencilState(pPipeline);
        stRendererVulkanPipelineSetupRasterizationState(pPipeline, VK_POLYGON_MODE_FILL);
	stRendererVulkanPipelineSetupVertexInputState(pPipeline);
        stRendererVulkanPipelineSetupInputAssemblyState(pPipeline, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        stRendererVulkanPipelineSetupLayout(pPipeline, pRange, range_count);
        VK_CHECK(vkCreatePipelineLayout(pBackend->device.logical_device, &pPipeline->info.layout_info, pBackend->pAllocator, &pPipeline->layout));

	for (int i = 0; i < shader_count; i++) {
		usize size = 0;
		void *buf = loadShaderFile(shader_paths[i], &size);
		STUPID_NC(buf);

		VkShaderModuleCreateInfo module_info = {0};
		module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		module_info.pCode = buf;
		module_info.codeSize = size;

		VkShaderModule module;
		vkCreateShaderModule(pBackend->device.logical_device, &module_info, pBackend->pAllocator, &module);

		stMemAppend(pPipeline->info.pShaderData, buf);

		VkPipelineShaderStageCreateInfo shader_info = {0};
		shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_info.pName = "main";
		shader_info.stage = shader_stages[i];
		shader_info.module = module;
		stMemAppend(pShaderStageInfo, shader_info);

		STUPID_LOG_DEBUG("loaded shader '%s'", shader_paths[i]);
	}

        stRendererVulkanPipelineSetupGraphicsPipeline(pPipeline, pShaderStageInfo, shader_count, pColorFormats, color_format_count, depth_format);

        VK_CHECK(vkCreateGraphicsPipelines(pBackend->device.logical_device,
                                           VK_NULL_HANDLE,
                                           1,
                                           &pPipeline->info.graphics_info,
                                           pBackend->pAllocator,
                                           &pPipeline->handle));

	for (int i = 0; i < shader_count; i++)
		vkDestroyShaderModule(pBackend->device.logical_device, pShaderStageInfo[i].module, pBackend->pAllocator);

	stMemDealloc(pShaderStageInfo);
}

void stRendererVulkanPipelineDestroy(StRendererVulkanBackend *pBackend, StRendererVulkanPipeline *pPipeline)
{
        STUPID_NC(pPipeline);

        vkDestroyPipeline(pBackend->device.logical_device, pPipeline->handle, pBackend->pAllocator);
        vkDestroyPipelineLayout(pBackend->device.logical_device, pPipeline->layout, pBackend->pAllocator);
	
	for (int i = 0; i < stMemLength(pPipeline->info.pShaderData); i++)
		stMemDealloc(pPipeline->info.pShaderData[i]);

	stMemDealloc(pPipeline->info.pDynamicStates);
	stMemDealloc(pPipeline->info.pShaderData);
}

void stRendererVulkanPipelineCreateCompute(StRendererVulkanBackend *pBackend, const char *shader_path, VkPushConstantRange *pRanges, const u32 range_count, StRendererVulkanPipeline *pPipeline)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(shader_path);
	STUPID_NC(pPipeline);

        stMemset(pPipeline, 0, sizeof(StRendererVulkanPipeline));

	usize size = 0;
	void *buf = loadShaderFile(shader_path, &size);
	STUPID_NC(buf);
	VkShaderModuleCreateInfo module_info = {0};
	module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_info.pCode = buf;
	module_info.codeSize = size;
	VkShaderModule compute_module;
	VK_CHECK(vkCreateShaderModule(pBackend->device.logical_device, &module_info, pBackend->pAllocator, &compute_module));

        pPipeline->info.pDynamicStates = stMemAlloc(VkDynamicState, 2);
        pPipeline->info.pShaderData = stMemAlloc(void *, 1);
	stMemAppend(pPipeline->info.pShaderData, buf);

	VkPipelineShaderStageCreateInfo stage_info = {0};
	stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stage_info.module = compute_module;
	stage_info.pName = "main";

	VkPipelineLayoutCreateInfo layout_info = {0};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_info.pPushConstantRanges = pRanges;
	layout_info.pushConstantRangeCount = 1;
	VK_CHECK(vkCreatePipelineLayout(pBackend->device.logical_device, &layout_info, pBackend->pAllocator, &pPipeline->layout));

	VkComputePipelineCreateInfo pipeline_info = {0};
	pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipeline_info.stage = stage_info;
	pipeline_info.layout = pPipeline->layout;
	VK_CHECK(vkCreateComputePipelines(pBackend->device.logical_device, VK_NULL_HANDLE, 1, &pipeline_info, pBackend->pAllocator, &pPipeline->handle));

	vkDestroyShaderModule(pBackend->device.logical_device, compute_module, pBackend->pAllocator);
}

