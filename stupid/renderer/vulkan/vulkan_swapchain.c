#include "common.h"

#include "vulkan_swapchain.h"
#include "vulkan_device.h"
#include "vulkan_image.h"
#include "vulkan_utils.h"

#include "core/logger.h"
#include "core/window.h"

#include "memory/memory.h"
#include <vulkan/vulkan_core.h>

//StRendererVulkanSwapchain *stRendererVulkanSwapchainCreate(StRendererVulkanBackend *pBackend, StRendererVulkanSurface *pSurface, VkPresentModeKHR mode, const u32 width, const u32 height)
bool stRendererVulkanSwapchainCreate(StRendererVulkanBackend *pBackend, VkSurfaceKHR surface, VkPresentModeKHR mode, const u32 width, const u32 height, StRendererVulkanSwapchain *pSwapchain)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);

	STUPID_ASSERT(width <= STUPID_WINDOW_MAX_WIDTH, "window width out of range");
	STUPID_ASSERT(height <= STUPID_WINDOW_MAX_HEIGHT, "window height out of range");

	// make sure there are no active gpu operations
	vkDeviceWaitIdle(pBackend->device.logical_device);

	// pSwapchain->dimensions
	VkExtent2D extent = { width, height };
	pSwapchain->swapchain_width = width;
	pSwapchain->swapchain_height = height;

	bool colorspace_found = false;

	// try to find a decent colorspace
	for (int i = 0; i < pBackend->device.swapchain_support.format_count; i++) {
		const VkSurfaceFormatKHR surface_format = pBackend->device.swapchain_support.pFormats[i];
		if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			pSwapchain->image_format = surface_format;
			colorspace_found = true;
			break;
		}
	}

	// accept the crappy one if it cant be found
	if (!colorspace_found)
		pSwapchain->image_format = pBackend->device.swapchain_support.pFormats[0];

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

	// try to find the requested present mode
	for (int i = 0; i < pBackend->device.swapchain_support.present_mode_count; i++) {
		if (pBackend->device.swapchain_support.pPresentModes[i] == mode) {
			present_mode = pBackend->device.swapchain_support.pPresentModes[i];
			break;
		}
		else if (pBackend->device.swapchain_support.pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			present_mode = pBackend->device.swapchain_support.pPresentModes[i];
		}
	}
	pSwapchain->present_mode = present_mode;

	stRendererVulkanDeviceQuerySwapchainSupport(&pBackend->device.swapchain_support, pBackend->device.physical_device, surface);

	// if this is not 0xFFFFFFFF then the extent must be capabilities.currentExtent for whatever reason
	if (pBackend->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX)
		extent = pBackend->device.swapchain_support.capabilities.currentExtent;

	const VkExtent2D min = pBackend->device.swapchain_support.capabilities.minImageExtent;
	const VkExtent2D max = pBackend->device.swapchain_support.capabilities.maxImageExtent;
	extent.width = STUPID_CLAMP(extent.width, min.width, max.width);
	extent.height = STUPID_CLAMP(extent.height, min.height, max.height);

	// the number of images in the swapchain
	u32 image_count = STUPID_MIN(pBackend->device.swapchain_support.capabilities.minImageCount + 1, 4);

	pSwapchain->max_frames_in_flight = image_count - 1;

	VkSwapchainCreateInfoKHR swapchain_info = {0};
	swapchain_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface          = surface;
	swapchain_info.minImageCount    = image_count; // images to buffer (i.e. 2 for double buffering)
	swapchain_info.imageFormat      = pSwapchain->image_format.format;
	swapchain_info.imageColorSpace  = pSwapchain->image_format.colorSpace;
	swapchain_info.imageExtent      = extent;
	swapchain_info.imageArrayLayers = 1; // the number of layers each image consists of (always 1 unless using stereoscopic 3D aka VR)
	swapchain_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // things the image is capable of doing (color for drawing and transfer dst for changing the format)

	if (pBackend->device.graphics_queue_index != pBackend->device.present_queue_index) {
		u32 pQueueFamilyIndices[2]     = {
			(u32)pBackend->device.graphics_queue_index,
			(u32)pBackend->device.present_queue_index
		};

		swapchain_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT; // if the graphics queue index and present queue index are separate then enable concurrent image sharing
		swapchain_info.queueFamilyIndexCount = 2;
		swapchain_info.pQueueFamilyIndices   = pQueueFamilyIndices;
	}
	else {
		swapchain_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE; // otherwise use non-concurrent image sharing
		swapchain_info.queueFamilyIndexCount = 0;
		swapchain_info.pQueueFamilyIndices   = NULL;
	}

	swapchain_info.preTransform   = pBackend->device.swapchain_support.capabilities.currentTransform; // orientation of the pSwapchain->(useless)
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // i think this is just the opacity
	swapchain_info.presentMode    = present_mode;
	swapchain_info.clipped	      = VK_TRUE; // whether Vulkan is allowed to skip rendering things not on the screen

	vkDeviceWaitIdle(pBackend->device.logical_device);
	VK_CHECK(vkCreateSwapchainKHR(pBackend->device.logical_device, &swapchain_info, pBackend->pAllocator, &pSwapchain->handle));
	VK_CHECK(vkGetSwapchainImagesKHR(pBackend->device.logical_device, pSwapchain->handle, &pSwapchain->image_count, NULL));

	pSwapchain->current_frame = 0;

	VkImage tmp[4] = {0};
	VK_CHECK(vkGetSwapchainImagesKHR(pBackend->device.logical_device, pSwapchain->handle, &pSwapchain->image_count, tmp));

	for (int i = 0; i < pSwapchain->image_count; i++) {
		VkImageViewCreateInfo view_info = {0};
		view_info.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image                       = tmp[i];
		view_info.viewType                    = VK_IMAGE_VIEW_TYPE_2D; // image view type (dont change unless using stereoscopic 3D)
		view_info.format                      = pSwapchain->image_format.format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.levelCount = 1; // the number of levels to use in mip mapping (left at 1 for now)
		view_info.subresourceRange.layerCount = 1; // the number of image array levels to use (should be left at 1 unless using stereoscopic 3D)

		// make sure the swapchain isnt in use right now
		vkDeviceWaitIdle(pBackend->device.logical_device);
		VK_CHECK(vkCreateImageView(pBackend->device.logical_device, &view_info, pBackend->pAllocator, &pSwapchain->pImages[i].view));

		stRendererVulkanImageCreateSwapchain(tmp[i], pSwapchain->pImages[i].view, width, height, pSwapchain->image_format.format, &pSwapchain->pImages[i]);
	}

	// try to get a valid Vulkan depth format
	if (!stRendererVulkanDeviceGetDepthFormat(&pBackend->device)) {
		pBackend->device.depth_format = VK_FORMAT_UNDEFINED;
		STUPID_LOG_FATAL("failed to find a supported depth format");
		return false;
	}

	stRendererVulkanImageCreateDepth(pBackend, true, pSwapchain->swapchain_width, pSwapchain->swapchain_height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &pSwapchain->depth_attachment);

	STUPID_LOG_TRACE("created pSwapchain->%p", pSwapchain->handle);

	pSwapchain->surface = surface;

	return pSwapchain;
}

void stRendererVulkanSwapchainDestroy(StRendererVulkanBackend *pBackend, StRendererVulkanSwapchain *pSwapchain)
{
	STUPID_NC(pSwapchain);

	// make sure the pSwapchain->isnt in use right now
	vkDeviceWaitIdle(pBackend->device.logical_device);

	stRendererVulkanImageDestroy(pBackend, &pSwapchain->depth_attachment);

	// destroy all the pSwapchain->image views
	for (int i = 0; i < pSwapchain->image_count; i++)
		vkDestroyImageView(pBackend->device.logical_device, pSwapchain->pImages[i].view, pBackend->pAllocator);

	// kill the swapchain
	vkDestroySwapchainKHR(pBackend->device.logical_device, pSwapchain->handle, pBackend->pAllocator);

	STUPID_LOG_TRACE("destroyed pSwapchain->%p", pSwapchain->handle);
}

//bool stRendererVulkanSwapchainRecreate(StRendererVulkanBackend *pBackend, StRendererVulkanSwapchain *pSwapchain, StRendererVulkanSurface *pSurface, const u32 width, const u32 height)
bool stRendererVulkanSwapchainRecreate(StRendererVulkanBackend *pBackend, const u32 width, const u32 height, StRendererVulkanSwapchain *pSwapchain)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pSwapchain);

	STUPID_ASSERT(width <= STUPID_WINDOW_MAX_WIDTH, "window width out of range");
	STUPID_ASSERT(height <= STUPID_WINDOW_MAX_HEIGHT, "window height out of range");

	// make sure there are no active gpu operations
	vkDeviceWaitIdle(pBackend->device.logical_device);

	// swapchain dimensions
	VkExtent2D extent = { width, height };
	pSwapchain->swapchain_width  = width;
	pSwapchain->swapchain_height = height;

	if (!stRendererVulkanDeviceQuerySwapchainSupport(&pBackend->device.swapchain_support, pBackend->device.physical_device, pSwapchain->surface)) return false;

	// if this is not 0xFFFFFFFF then the extent must be capabilities.currentExtent for whatever reason
	if (pBackend->device.swapchain_support.capabilities.currentExtent.width != UINT32_MAX)
		extent = pBackend->device.swapchain_support.capabilities.currentExtent;

	const VkExtent2D min = pBackend->device.swapchain_support.capabilities.minImageExtent;
	const VkExtent2D max = pBackend->device.swapchain_support.capabilities.maxImageExtent;
	extent.width = STUPID_CLAMP(extent.width, min.width, max.width);
	extent.height = STUPID_CLAMP(extent.height, min.height, max.height);

	// the number of images in the swapchain
	u32 image_count = STUPID_MIN(pBackend->device.swapchain_support.capabilities.minImageCount + 1, 4);

	VkSwapchainCreateInfoKHR swapchain_info = {0};
	swapchain_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.oldSwapchain     = pSwapchain->handle;
	swapchain_info.surface          = pSwapchain->surface;
	swapchain_info.minImageCount    = image_count; // images to buffer (i.e. 2 for double buffering)
	swapchain_info.imageFormat      = pSwapchain->image_format.format;
	swapchain_info.imageColorSpace  = pSwapchain->image_format.colorSpace;
	swapchain_info.imageExtent      = extent;
	swapchain_info.imageArrayLayers = 1; // the number of layers each image consists of (always 1 unless using stereoscopic 3D aka VR)
	swapchain_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // things the image is capable of doing (color for drawing and transfer dst for changing the format)

	if (pBackend->device.graphics_queue_index != pBackend->device.present_queue_index) {
		const u32 pQueueFamilyIndices[2] = {
			(u32)pBackend->device.graphics_queue_index,
			(u32)pBackend->device.present_queue_index
		};

		swapchain_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT; // if the graphics queue index and present queue index are separate then enable concurrent image sharing
		swapchain_info.queueFamilyIndexCount = 2;
		swapchain_info.pQueueFamilyIndices   = pQueueFamilyIndices;
	}
	else {
		swapchain_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE; // otherwise use non-concurrent image sharing
		swapchain_info.queueFamilyIndexCount = 0;
		swapchain_info.pQueueFamilyIndices   = NULL;
	}

	swapchain_info.preTransform   = pBackend->device.swapchain_support.capabilities.currentTransform; // orientation of the swapchain (useless)
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode    = pSwapchain->present_mode;
	swapchain_info.clipped	      = VK_TRUE; // whether vulkan is allowed to skip rendering things not on the screen

	VkSwapchainKHR tmp_swapchain = VK_NULL_HANDLE;
	VK_CHECK(vkCreateSwapchainKHR(pBackend->device.logical_device, &swapchain_info, pBackend->pAllocator, &tmp_swapchain));
	for (int i = 0; i < image_count; i++)
		vkDestroyImageView(pBackend->device.logical_device, pSwapchain->pImages[i].view, pBackend->pAllocator);
	

	VkSwapchainKHR tmp_pointer = pSwapchain->handle;

	vkDestroySwapchainKHR(pBackend->device.logical_device, pSwapchain->handle, pBackend->pAllocator);
	pSwapchain->handle = tmp_swapchain;
	VK_CHECK(vkGetSwapchainImagesKHR(pBackend->device.logical_device,
					 pSwapchain->handle,
					 &pSwapchain->image_count,
					 NULL));

	if (pSwapchain->image_count > stMemCapacity(pSwapchain->pImages))
		stMemResize(pSwapchain->pImages, pSwapchain->image_count);

	VkImage tmp[4] = {0};
	VK_CHECK(vkGetSwapchainImagesKHR(pBackend->device.logical_device,
					 pSwapchain->handle,
					 &pSwapchain->image_count,
					 tmp));

	pSwapchain->current_frame = 0;

	for (int i = 0; i < pSwapchain->image_count; i++) {
		VkImageViewCreateInfo view_info = {0};
		view_info.sType	                      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image	                      = tmp[i];
		view_info.viewType                    = VK_IMAGE_VIEW_TYPE_2D; // image view type (dont change unless using stereoscopic 3D aka VR)
		view_info.format                      = pSwapchain->image_format.format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.levelCount = 1; // the number of levels to use in mip mapping (left at 1 for now)
		view_info.subresourceRange.layerCount = 1; // the number of image array levels to use (should be left at 1 unless using stereoscopic 3D aka VR)

		// make sure the swapchain isnt in use right now
		vkDeviceWaitIdle(pBackend->device.logical_device);
		VK_CHECK(vkCreateImageView(pBackend->device.logical_device, &view_info, pBackend->pAllocator, &pSwapchain->pImages[i].view));

		stRendererVulkanImageCreateSwapchain(tmp[i], pSwapchain->pImages[i].view, width, height, pSwapchain->image_format.format, &pSwapchain->pImages[i]);
	}

	stRendererVulkanImageDestroy(pBackend, &pSwapchain->depth_attachment);
	stRendererVulkanImageCreateDepth(pBackend, true, pSwapchain->swapchain_width, pSwapchain->swapchain_height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &pSwapchain->depth_attachment);

	STUPID_LOG_TRACE("recreated swapchain %p -> %p", tmp_pointer, pSwapchain->handle);

	return true;
}

bool stRendererVulkanSwapchainAcquireNextImageIndex(StRendererVulkanContext *pContext, const u64 timeout_ns, VkFence fence)
{
	// attempt to acquire the next image from the swapchain
	const VkResult result = vkAcquireNextImageKHR(pContext->pBackend->device.logical_device,
						      pContext->swapchain.handle,
						      timeout_ns,
						      pContext->pImageAvailableSemaphores[pContext->current_frame],
						      fence,
						      &pContext->image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		stRendererVulkanSwapchainRecreate(pContext->pBackend, pContext->swapchain.swapchain_width, pContext->swapchain.swapchain_height, &pContext->swapchain);
		return false;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		STUPID_LOG_FATAL("failed to acquire the next swapchain image");
		return false;
	}

	pContext->pRenderingAttachments[0].imageView = pContext->swapchain.pImages[pContext->image_index].view;

	return true;
}

bool stRendererVulkanSwapchainPresent(StRendererVulkanContext *pContext, VkSemaphore render_complete_semaphore, const u32 present_image_index)
{
	// information about presenting the swapchain image
	VkPresentInfoKHR present_info = {0};
	present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores    = &render_complete_semaphore;
	present_info.swapchainCount     = 1;
	present_info.pSwapchains        = &pContext->swapchain.handle;
	present_info.pImageIndices      = &present_image_index;

	pContext->current_frame = (pContext->current_frame + 1) % pContext->swapchain.max_frames_in_flight;

	const VkResult result = vkQueuePresentKHR(pContext->pBackend->device.present_queue, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		stRendererVulkanSwapchainRecreate(pContext->pBackend, pContext->swapchain.swapchain_width, pContext->swapchain.swapchain_height, &pContext->swapchain);
		return false;
	}
	else if (!stRendererVulkanResultIsSuccess(result)) {
		STUPID_LOG_ERROR("failed to present the worthless swapchain image: %u", present_image_index);
		return false;
	}

	return true;
}

