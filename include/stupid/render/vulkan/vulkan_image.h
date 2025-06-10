#pragma once

#include "stupid/render/vulkan/vulkan_types.h"
#include "stupid/render/vulkan/vulkan_utils.h"

#include "stupid/common.h"
#include "stupid/assert.h"
#include "stupid/logger.h"

/**
 * Converts an StRendererVulkanImage to the specified layout.
 * @param cmd An active command buffer.
 * @param new_layout The image layout to change the StRendererVulkanImage to.
 * @param pImage Pointer to the image to be converted.
 * @see StRendererVulkanImage, VkImageLayout
 */
void stRendererVulkanImageConvert(VkCommandBuffer cmd, const VkImageLayout new_layout, StRendererVulkanImage *pImage);

/**
 * Creates a StRendererVulkanImage based on the configuration inside of the output StRendererVulkanImage.
 * @param pBackend Pointer to a vulkan backend.
 * @see stRendererVulkanImageDestroy
 */
void stRendererVulkanImageCreate(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage);

/**
 * Creates a vulkan color image based on a preset for typical usecases.
 * @param pBackend Pointer to a vulkan backend.
 * @param create_view Whether an image view should also be created.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param usage Things the image will be used for.
 * @param pImage Output image.
 * @see stRendererVulkanImageCreateDepth, stRendererVulkanImageDestroy
 */
static STUPID_INLINE void stRendererVulkanImageCreateColor(const StRendererVulkanBackend *pBackend, bool create_view, const u32 width, const u32 height, const VkImageUsageFlagBits usage, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pImage);
	pImage->options.width        = width;
	pImage->options.height       = height;
	pImage->options.format       = VK_FORMAT_R8G8B8A8_UNORM;
	pImage->options.usage_flags  = usage;
	pImage->options.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	pImage->options.type         = VK_IMAGE_TYPE_2D;
	pImage->options.view_type    = VK_IMAGE_VIEW_TYPE_2D;
	pImage->options.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
	pImage->options.view         = create_view;
	pImage->layout               = VK_IMAGE_LAYOUT_UNDEFINED;
	stRendererVulkanImageCreate(pBackend, pImage);
}

/**
 * Creates a vulkan depth image based on a preset for typical usecases.
 * @param pBackend Pointer to a vulkan backend.
 * @param create_view Whether an image view should also be created.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param usage Things the image will be used for.
 * @param pImage Output image.
 * @see stRendererVulkanImageCreateColor, stRendererVulkanImageDestroy
 */
static STUPID_INLINE void stRendererVulkanImageCreateDepth(const StRendererVulkanBackend *pBackend, bool create_view, const u32 width, const u32 height, const VkImageUsageFlagBits usage, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pImage);
	pImage->options.width        = width;
	pImage->options.height       = height;
	pImage->options.format       = pBackend->device.depth_format;
	pImage->options.usage_flags  = usage;
	pImage->options.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	pImage->options.type         = VK_IMAGE_TYPE_2D;
	pImage->options.view_type    = VK_IMAGE_VIEW_TYPE_2D;
	pImage->options.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	pImage->options.view         = create_view;
	pImage->layout               = VK_IMAGE_LAYOUT_UNDEFINED;
	stRendererVulkanImageCreate(pBackend, pImage);
}

/**
 * Destroys a vulkan image.
 * @param pBackend Pointer to a vulkan backend.
 * @param pImage Image to destroy.
 * @see stRendererVulkanImageCreate
 */
static STUPID_INLINE void stRendererVulkanImageDestroy(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pImage);
	if (pImage->view != VK_NULL_HANDLE)
		vkDestroyImageView(pBackend->device.logical_device, pImage->view, pBackend->pAllocator);
	vkDestroyImage(pBackend->device.logical_device, pImage->handle, pBackend->pAllocator);
	vkFreeMemory(pBackend->device.logical_device, pImage->memory, pBackend->pAllocator);
}

/**
 * Recreates a vulkan image based on pImage->options.
 * @param pBackend Pointer to a vulkan backend.
 * @param pImage Image to recreate.
 * @see stRendererVulkanImageCreate
 */
static STUPID_INLINE void stRendererVulkanImageRecreate(const StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pImage);
	stRendererVulkanImageDestroy(pBackend, pImage);
	stRendererVulkanImageCreate(pBackend, pImage);
}

/**
 * Resizes a vulkan image.
 * @param pBackend Pointer to a vulkan backend.
 * @param width New width.
 * @param height New height.
 * @param pImage Image to resize.
 */
static STUPID_INLINE void stRendererVulkanImageResize(const StRendererVulkanBackend *pBackend, const u32 width, const u32 height, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pImage);

	STUPID_ASSERT(width != 0, "invalid height");
	STUPID_ASSERT(height != 0, "invalid height");

	pImage->options.width = width;
	pImage->options.height = height;
	stRendererVulkanImageRecreate(pBackend, pImage);
}

/**
 * Draws a vulkan color image on top of another vulkan color image with the specified dimensions and position.
 * @param pBackend Pointer to a vulkan backend.
 * @param x Output x position.
 * @param y Output y position.
 * @param w Output width.
 * @param h Output height.
 * @param pImage Source image.
 * @param pOutput Destination image.
 */
bool stRendererVulkanImageBlit(StRendererVulkanBackend *pBackend, VkCommandBuffer cmd, i32 x, i32 y, u32 w, u32 h, const StRendererVulkanImage *pImage, StRendererVulkanImage *pOutput);

/**
 * Creates an StRendererVulkanImage from a vulkan swapchain image handle.
 * @param pBackend Pointer to a vulkan backend.
 * @param handle Swapchain image handle.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param format Format of the image.
 * @param pImage Output image.
 * @see stRendererVulkanImageCreate
 */
static STUPID_INLINE void stRendererVulkanImageCreateSwapchain(const StRendererVulkanBackend *pBackend, const VkImage handle, const u32 width, const u32 height, const VkFormat format, StRendererVulkanImage *pImage)
{
	STUPID_NC(pImage);
	pImage->layout            = VK_IMAGE_LAYOUT_UNDEFINED;
	pImage->handle            = handle;
	pImage->options.type      = VK_IMAGE_TYPE_2D;
	pImage->options.view_type = VK_IMAGE_VIEW_TYPE_2D;
	pImage->options.format    = format;
	pImage->options.width     = width;
	pImage->options.height    = height;
	pImage->options.view      = true;

	VkImageViewCreateInfo view_info       = {0};
	view_info.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image                       = pImage->handle;
	view_info.viewType                    = pImage->options.view_type; // image view type (dont change unless using stereoscopic 3D)
	view_info.format                      = format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.levelCount = 1; // the number of levels to use in mip mapping (left at 1 for now)
	view_info.subresourceRange.layerCount = 1; // the number of image array levels to use (should be left at 1 unless using stereoscopic 3D)

	vkDeviceWaitIdle(pBackend->device.logical_device);
	VK_CHECK(vkCreateImageView(pBackend->device.logical_device, &view_info, pBackend->pAllocator, &pImage->view));
}

/**
 * Clears a vulkan color image.
 * @param cmd Command buffer to execute this on.
 * @param color Color to set the image to.
 * @param pImage Image to clear.
 */
void stRendererVulkanImageClear(VkCommandBuffer cmd, const StColor color, StRendererVulkanImage *pImage);

/**
 * @brief Maps a vulkan image to memory.
 * When mapped, the image layout in memory is based on the image format.
 * For example, a VK_FORMAT_R8G8B8A8UNORM image in memory would be [u8 red, u8 green, u8 blue, u8 alpha] * pixel count.
 * @param pBackend Pointer to a vulkan backend.
 * @param map Pointer to store the location of the mapped memory in.
 * @param pImage Image to map to memory.
 */
static STUPID_INLINE void stRendererVulkanImageMap(StRendererVulkanBackend *pBackend, void **map, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pImage);
	STUPID_NC(map);

	vkMapMemory(pBackend->device.logical_device, pImage->memory, 0, pImage->size, 0, map);
}

/**
 * Unmaps a vulkan image from memory.
 * @param pBackend Pointer to a vulkan backend.
 * @param pImage Image to unmap from memory.
 */
static STUPID_INLINE void stRendererVulkanImageUnmap(StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pImage);

	vkUnmapMemory(pBackend->device.logical_device, pImage->memory);
}

