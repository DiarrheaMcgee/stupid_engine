#include "common.h"

#include "vulkan_image.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan_utils.h"
#include "vulkan_memory.h"
#include "vulkan_command_buffer.h"
#include "core/logger.h"

#include "memory/memory.h"
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#include "stb/stb_image.h"

void stRendererVulkanImageConvert(VkCommandBuffer command_buffer, const VkImageLayout new_layout,  StRendererVulkanImage *pImage)
{
	STUPID_NC(pImage);

	if (pImage->layout == new_layout) return;

	// TODO: setup StRendererVulkan Synchronization 2
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
	image_info.mipLevels	 = 4;
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
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = pImage->handle;
		view_info.format = pImage->options.format;
		view_info.subresourceRange.aspectMask = pImage->options.aspect_flags;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.layerCount = 1;
		switch (pImage->options.type) {
		case VK_IMAGE_TYPE_1D:
			view_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
			break;
		case VK_IMAGE_TYPE_2D:
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case VK_IMAGE_TYPE_3D:
			view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
			break;
		default:
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		}
		view_info.format = pImage->options.format;
		view_info.subresourceRange.aspectMask = pImage->options.aspect_flags;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(pBackend->device.logical_device, &view_info, pBackend->pAllocator, &pImage->view));
	}
}

void stRendererVulkanImageDestroy(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pImage);
	if (pImage->view != VK_NULL_HANDLE)
		vkDestroyImageView(pBackend->device.logical_device, pImage->view, pBackend->pAllocator);
	vkDestroyImage(pBackend->device.logical_device, pImage->handle, pBackend->pAllocator);
	vkFreeMemory(pBackend->device.logical_device, pImage->memory, pBackend->pAllocator);
}

void stRendererVulkanImageRecreate(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage)
{
	STUPID_NC(pImage);
	stRendererVulkanImageDestroy(pBackend, pImage);
	stRendererVulkanImageCreate(pBackend, pImage);
}

bool stRendererVulkanImageLoad(StRendererVulkanContext *pContext, const char *path, const u32 x, const u32 y, const u32 w, const u32 h, StRendererVulkanImage *pImage)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);
	STUPID_NC(path);
	STUPID_NC(pImage);

	i32 width = 0, height = 0;
	void *data = NULL;

	int channels = 0;
	data = stbi_load(path, &width, &height, &channels, 4);
	if (data == NULL) {
		STUPID_LOG_ERROR("invalid image '%s'", path);
		return false;
	}

	usize image_size = width * height * 4;

	StRendererVulkanBuffer buffer = {0};
	stRendererVulkanMemoryAllocate(pContext->pBackend, width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer);
	void *out = NULL;
	stRendererVulkanMemoryMap(pContext->pBackend, &out, &buffer);
	stMemcpy(out, data, image_size);
	stRendererVulkanMemoryUnmap(pContext->pBackend, &buffer);
	stbi_image_free(data);

	stRendererVulkanImageCreateColor(pContext->pBackend, false, width, height, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, pImage);

	VkBufferImageCopy region = {0};
	region.bufferRowLength = width;
	region.bufferImageHeight = height;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageOffset.x = 0;
	region.imageOffset.y = 0;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;

	StRendererVulkanCommandBuffer cmd = {0};
	stRendererVulkanCommandBufferBeginTemporary(pContext, pContext->pBackend->device.graphics_command_pool, &cmd);

	stRendererVulkanImageConvert(cmd.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pImage);

	vkCmdCopyBufferToImage(cmd.handle,
			       buffer.handle,
			       pImage->handle,
			       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			       1,
			       &region);

	stRendererVulkanImageConvert(cmd.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pImage);
	stRendererVulkanCommandBufferEndTemporary(pContext, pContext->pBackend->device.graphics_command_pool, &cmd, pContext->pBackend->device.graphics_queue);
	stRendererVulkanMemoryDeallocate(pContext->pBackend, &buffer);

	return true;
}

bool stRendererVulkanImageBlit(StRendererVulkanBackend *pBackend, VkCommandBuffer cmd, i32 x, i32 y, u32 w, u32 h, const StRendererVulkanImage *pImage, StRendererVulkanImage *pOutput)
{
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

/*void stRendererVulkanImageCreate(StRendererVulkanBackend *pBackend,
		       VkImageType image_type,
		       const u32 width,
		       const u32 height,
		       const VkFormat format,
		       const VkImageUsageFlags usage,
		       const VkMemoryPropertyFlags memory_flags,
		       const bool create_view,
		       const VkImageAspectFlags view_aspect_flags,
		       StRendererVulkanImage *pImage)
{
	// specify the image dimensions
	pImage->width  = width;
	pImage->height = height;

	// Vulkan image struct
	VkImageCreateInfo image_info = {0};

	// Vulkan structure type
	image_info.sType	 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

	// the type of Vulkan image (dont change unless using stereoscopic 3D (VR))
	image_info.imageType	 = VK_IMAGE_TYPE_2D;

	// width of the Vulkan image
	image_info.extent.width  = width;

	// height of the Vulkan image
	image_info.extent.height = height;

	// depth of the Vulkan image
	image_info.extent.depth  = 1;

	// number of mip levels for mip mapping
	image_info.mipLevels	 = 4;

	// number of Vulkan array layers in the image (should be left at 1 unless using stereoscopic 3D (VR))
	image_info.arrayLayers	 = 1;

	// format for the Vulkan image (i.e. VK_FORMAT_B8R8G8STUPID_A8_SRGB for a color image or VK_FORMAT_D32_SFLOAT for a depth image)
	image_info.format	 = format;

	// image tiling method (should be left as VK_IMAGE_TILING_OPTIMAL to let the driver decide)
	image_info.tiling	 = VK_IMAGE_TILING_OPTIMAL;

	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // doesnt matter

	// usage of the image (i.e. VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT for a color image)
	image_info.usage	 = usage;

	// number of Vulkan samples for multisampling (left as VK_SAMPLE_COUNT_1_BIT because multisampling isnt a thing yet)
	image_info.samples	 = VK_SAMPLE_COUNT_1_BIT;

	// Vulkan image sharing mode (should be left as VK_SHARING_MODE_EXCLUSIVE)
	image_info.sharingMode	 = VK_SHARING_MODE_EXCLUSIVE;

	// make sure the swapchain isnt in use right now
	vkDeviceWaitIdle(pBackend->device.logical_device);

	// attempt to create the Vulkan image based on the struct
	VK_CHECK(vkCreateImage(pBackend->device.logical_device, &image_info, pBackend->pAllocator, &pImage->handle));

	// Vulkan memory requirements for the image just created
	VkMemoryRequirements memory_requirements = {0};

	// get the Vulkan image memory requirements of the image
	vkGetImageMemoryRequirements(pBackend->device.logical_device, pImage->handle, &memory_requirements);

	// get the memory type
	const i32 memory_type = stRendererVulkanMemoryGetIndex(pBackend, memory_requirements.memoryTypeBits, memory_flags);
	if (memory_type == -1)
		STUPID_LOG_ERROR("required memory type not found dag nabbit the stupid image isnt valid");

	// Vulkan memory allocation information
	VkMemoryAllocateInfo allocate_info = {0};

	// Vulkan structure type
	allocate_info.sType	      = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	// the allocation size (how much memory to allocate for the object
	allocate_info.allocationSize  = memory_requirements.size;

	// the type of memory to use
	allocate_info.memoryTypeIndex = memory_type;

	// make sure the swapchain isnt in use right now
	vkDeviceWaitIdle(pBackend->device.logical_device);

	// attempt to allocate the memory for the image
	VK_CHECK(vkAllocateMemory(pBackend->device.logical_device, &allocate_info, pBackend->pAllocator, &pImage->memory));

	// make sure the swapchain isnt in use right now
	vkDeviceWaitIdle(pBackend->device.logical_device);

	// attempt to bind the memory to the image
	VK_CHECK(vkBindImageMemory(pBackend->device.logical_device, pImage->handle, pImage->memory, 0));

	// create the image view for the image if told to
	if (create_view) {
		pImage->view = NULL;
		VkImageViewType type = 0;
		switch (image_type) {
		case VK_IMAGE_TYPE_1D:
			type = VK_IMAGE_VIEW_TYPE_1D;
			break;
		case VK_IMAGE_TYPE_2D:
			type = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case VK_IMAGE_TYPE_3D:
			type = VK_IMAGE_VIEW_TYPE_3D;
			break;
		default:
			type = VK_IMAGE_VIEW_TYPE_2D;
		}
		stRendererVulkanImageViewCreate(pBackend, type, format, pImage->handle, &pImage->view, view_aspect_flags);
	}
}

void stRendererVulkanImageDestroy(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage)
{
	// wait to make sure there are no active graphics operations
	vkDeviceWaitIdle(pBackend->device.logical_device);

	// destroy the Vulkan image view if it exists
	if (pImage->view != VK_NULL_HANDLE) {
		vkDestroyImageView(pBackend->device.logical_device, pImage->view, pBackend->pAllocator);
		pImage->view = NULL;
	}

	// free the Vulkan memory if it exists
	if (pImage->memory != NULL) {
		vkFreeMemory(pBackend->device.logical_device, pImage->memory, pBackend->pAllocator);
		pImage->memory = NULL;
	}

	// destroy the image if it exists
	if (pImage->handle != VK_NULL_HANDLE) {
		vkDestroyImage(pBackend->device.logical_device, pImage->handle, pBackend->pAllocator);
		pImage->handle = NULL;
	}
}*/

