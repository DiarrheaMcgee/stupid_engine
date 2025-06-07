#pragma once

#include "stupid/common.h"
#include "stupid/renderer/vulkan/vulkan_types.h"

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
				    const u32 range_count);

void stRendererVulkanPipelineDestroy(StRendererVulkanBackend *pBackend, StRendererVulkanPipeline *pPipeline);

void stRendererVulkanPipelineCreateCompute(StRendererVulkanBackend *pBackend, const char *shader_path, VkPushConstantRange *pRanges, const u32 range_count, StRendererVulkanPipeline *pPipeline);
