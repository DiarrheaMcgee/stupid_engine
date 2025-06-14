#include "stupid/render/vulkan/vulkan_image.h"
#include "stupid/render/vulkan/vulkan_types.h"
#include "stupid/render/vulkan/vulkan_utils.h"
#include "stupid/render/vulkan/vulkan_memory.h"
#include "stupid/render/vulkan/vulkan_command_buffer.h"

#include "stupid/common.h"
#include "stupid/logger.h"
#include "stupid/memory.h"
#include <vulkan/vulkan_core.h>

void stRendererVulkanImageConvert(VkCommandBuffer command_buffer, const VkImageLayout new_layout,  StRendererVulkanImage *pImage)
{
	STUPID_NC(pImage);

	if (pImage->layout == new_layout) return;

	// TODO: setup vulkan synchronization 2
	VkImageMemoryBarrier barrier = {0};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = pImage->layout;
	barrier.newLayout = new_layout;
	barrier.image = pImage->handle;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags src_stage = 0;
	VkPipelineStageFlags dest_stage = 0;

	switch (pImage->layout) {
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		src_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		src_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		src_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;

	default:
		src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		barrier.srcAccessMask = 0;
		break;
	}

	switch (new_layout) {
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		dest_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		dest_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		dest_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		break;

	default:
		dest_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		barrier.dstAccessMask = 0;
		break;
	}

	vkCmdPipelineBarrier(command_buffer,
			     src_stage,
			     dest_stage,
			     0,
			     0,
			     NULL,
			     0,
			     NULL,
			     1,
			     &barrier);

	pImage->layout = new_layout;
}

void stRendererVulkanImageCreate(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pImage);

	VkImageCreateInfo image_info = {0};
	image_info.sType	 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType	 = pImage->options.type;
	image_info.extent.width  = pImage->options.width;
	image_info.extent.height = pImage->options.height;
	image_info.extent.depth  = 1;
	image_info.mipLevels	 = 1;
	image_info.arrayLayers	 = 1;
	image_info.format	 = pImage->options.format;
	image_info.tiling	 = VK_IMAGE_TILING_OPTIMAL;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage	 = pImage->options.usage_flags;
	image_info.samples	 = VK_SAMPLE_COUNT_1_BIT;
	image_info.sharingMode	 = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateImage(pBackend->device.logical_device,
		      &image_info,
		      pBackend->pAllocator,
		      &pImage->handle);

	VkMemoryAllocateInfo allocate_info = {0};
	VkMemoryRequirements memory_requirements = {0};
	vkGetImageMemoryRequirements(pBackend->device.logical_device, pImage->handle, &memory_requirements);
	pImage->size = memory_requirements.size;

	const i32 memory_type = stRendererVulkanMemoryGetIndex(pBackend, memory_requirements.memoryTypeBits, pImage->options.memory_flags);

	allocate_info.sType	      = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize  = memory_requirements.size;
	allocate_info.memoryTypeIndex = memory_type;
	
	vkDeviceWaitIdle(pBackend->device.logical_device);
	VK_CHECK(vkAllocateMemory(pBackend->device.logical_device, &allocate_info, pBackend->pAllocator, &pImage->memory));
	vkDeviceWaitIdle(pBackend->device.logical_device);

	// attempt to bind the memory to the image
	VK_CHECK(vkBindImageMemory(pBackend->device.logical_device, pImage->handle, pImage->memory, 0));
	
	if (pImage->options.view) {
		VkImageViewCreateInfo view_info = {0};
		view_info.sType  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image  = pImage->handle;
		view_info.format = pImage->options.format;
		view_info.subresourceRange.aspectMask = pImage->options.aspect_flags;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.layerCount = 1;
		view_info.viewType = pImage->options.view_type;
		view_info.format   = pImage->options.format;
		view_info.subresourceRange.aspectMask = pImage->options.aspect_flags;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(pBackend->device.logical_device, &view_info, pBackend->pAllocator, &pImage->view));
	}
}

bool stRendererVulkanImageBlit(StRendererVulkanBackend *pBackend, VkCommandBuffer cmd, i32 x, i32 y, u32 w, u32 h, const StRendererVulkanImage *pImage, StRendererVulkanImage *pOutput)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pImage);
	STUPID_NC(pOutput);

	VkImageBlit region = {0};
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.layerCount = 1;
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.layerCount = 1;

	i32 src_w = pImage->options.width;
	i32 src_h = pImage->options.height;

	if (x >= pOutput->options.width || x + w <= 0) return false;
	if (y >= pOutput->options.height || y + h <= 0) return false;

	i32 dest_x0 = x < 0 ? 0 : x;
	i32 dest_y0 = y < 0 ? 0 : y;
	i32 dest_x1 = 0;
	i32 dest_y1 = 0;

	if (x + w > pOutput->options.width)
		dest_x1 = pOutput->options.width;
	else
		dest_x1 = x + w;

	if (y + h > pOutput->options.height)
		dest_y1 = pOutput->options.height;
	else
		dest_y1 = y + h;

	float src_scale_x = (float)pImage->options.width / (float)w;
	float src_scale_y = (float)pImage->options.height / (float)h;
	i32 src_x0 = (dest_x0 - x) * src_scale_x;
	i32 src_y0 = (dest_y0 - y) * src_scale_y;
	i32 src_x1 = src_x0 + (dest_x1 - dest_x0) * src_scale_x;
	i32 src_y1 = src_y0 + (dest_y1 - dest_y0) * src_scale_y;

	if (src_x0 < 0) src_x0 = 0;
	if (src_y0 < 0) src_y0 = 0;
	if (src_x1 > pImage->options.width) src_x1 = src_w;
	if (src_y1 > pImage->options.height) src_y1 = src_h;

	region.dstOffsets[0].x = dest_x0;
	region.dstOffsets[0].y = dest_y0;
	region.dstOffsets[1].x = dest_x1;
	region.dstOffsets[1].y = dest_y1;
	region.dstOffsets[1].z = 1;

	region.srcOffsets[0].x = src_x0;
	region.srcOffsets[0].y = src_y0;
	region.srcOffsets[1].x = src_x1;
	region.srcOffsets[1].y = src_y1;
	region.srcOffsets[1].z = 1;

	vkCmdBlitImage(cmd, pImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pOutput->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
	return true;
}

void stRendererVulkanImageClear(VkCommandBuffer cmd, const StColor color, StRendererVulkanImage *pImage)
{
        VkClearColorValue clear_color_value = {0};
        clear_color_value.float32[0] = color.r;
        clear_color_value.float32[1] = color.g;
        clear_color_value.float32[2] = color.b;
        clear_color_value.float32[3] = color.a;

        VkImageSubresourceRange subresource_range = {0};
        subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.baseMipLevel = 0;
        subresource_range.levelCount = 1;
        subresource_range.baseArrayLayer = 0;
        subresource_range.layerCount = 1;

	const VkImageLayout old_layout = pImage->layout;

	if (old_layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		stRendererVulkanImageConvert(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pImage);

	const VkImageLayout new_layout = pImage->layout;

        vkCmdClearColorImage(cmd,
	                     pImage->handle,
	                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                     &clear_color_value,
	                     1,
	                     &subresource_range);

	if (old_layout != new_layout && old_layout != VK_IMAGE_LAYOUT_UNDEFINED)
		stRendererVulkanImageConvert(cmd, old_layout, pImage);

}

