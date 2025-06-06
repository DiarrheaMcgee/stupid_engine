#pragma once

#include <vulkan/vulkan_core.h>
#include "common.h"
#include "core/asserts.h"
#include "vulkan_types.h"
#include "core/logger.h"

/**
 * Converts an StRendererVulkanImage to the specified layout.
 * @param cmd An active command buffer.
 * @param new_layout The image layout to change the StRendererVulkanImage to.
 * @param pImage Pointer to the image to be converted.
 */
void stRendererVulkanImageConvert(VkCommandBuffer cmd, const VkImageLayout new_layout, StRendererVulkanImage *pImage);

/**
 * Creates a StRendererVulkanImage based on the configuration inside of the output StRendererVulkanImage.
 * @param pBackend Pointer to the
 */
void stRendererVulkanImageCreate(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage);

static inline void stRendererVulkanImageCreateColor(const StRendererVulkanBackend *pBackend, bool create_view, const u32 width, const u32 height, const VkImageUsageFlagBits usage, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pImage);
	pImage->options.width        = width;
	pImage->options.height       = height;
	pImage->options.format       = VK_FORMAT_R8G8B8A8_UNORM;
	pImage->options.usage_flags  = usage;
	pImage->options.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	pImage->options.type         = VK_IMAGE_TYPE_2D;
	pImage->options.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	pImage->options.view         = create_view;
	pImage->layout               = VK_IMAGE_LAYOUT_UNDEFINED;
	stRendererVulkanImageCreate(pBackend, pImage);
}

static inline void stRendererVulkanImageCreateDepth(const StRendererVulkanBackend *pBackend, bool create_view, const u32 width, const u32 height, const VkImageUsageFlagBits usage, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pImage);
	pImage->options.width        = width;
	pImage->options.height       = height;
	pImage->options.format       = pBackend->device.depth_format;
	pImage->options.usage_flags  = usage;
	pImage->options.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	pImage->options.type         = VK_IMAGE_TYPE_2D;
	pImage->options.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	pImage->options.view         = create_view;
	pImage->layout               = VK_IMAGE_LAYOUT_UNDEFINED;
	stRendererVulkanImageCreate(pBackend, pImage);
}

void stRendererVulkanImageDestroy(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage);

void stRendererVulkanImageRecreate(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage);

static STUPID_INLINE void stRendererVulkanImageResize(const StRendererVulkanBackend *pBackend, const u32 width, const u32 height, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pImage);

	pImage->options.width = width;
	pImage->options.height = height;
	stRendererVulkanImageRecreate(pBackend, pImage);
}

bool stRendererVulkanImageLoad(StRendererVulkanContext *pContext, const char *path, const u32 x, const u32 y, const u32 w, const u32 h, StRendererVulkanImage *pImage);

bool stRendererVulkanImageBlit(StRendererVulkanBackend *pBackend, VkCommandBuffer cmd, i32 x, i32 y, u32 w, u32 h, const StRendererVulkanImage *pImage, StRendererVulkanImage *pOutput);

static inline void stRendererVulkanImageCreateSwapchain(const VkImage handle, const VkImageView view, const u32 width, const u32 height, const VkFormat format, StRendererVulkanImage *pImage)
{
	STUPID_NC(pImage);
	pImage->layout         = VK_IMAGE_LAYOUT_UNDEFINED;
	pImage->handle         = handle;
	pImage->view           = view;
	pImage->options.type   = VK_IMAGE_TYPE_2D;
	pImage->options.format = format;
	pImage->options.width  = width;
	pImage->options.height = height;
	pImage->options.view   = true;
}

static inline void stRendererVulkanImageClear(VkCommandBuffer cmd, const StColor color, StRendererVulkanImage *pImage)
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

