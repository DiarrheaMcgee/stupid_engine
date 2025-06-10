#pragma once

#include "stupid/common.h"
#include "stupid/render/vulkan/vulkan_types.h"

/**
 * Creates a vulkan swapchain.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param mode Present mode for the window surface.
 * @param width Width of the swapchain.
 * @param height Height of the swapchain.
 * @note If the present mode cant be found, VK_PRESENT_MODE_IMMEDIATE_KHR will be used.
 */
bool stRendererVulkanSwapchainCreate(StRendererVulkanBackend *pBackend, VkSurfaceKHR surface, VkPresentModeKHR mode, const u32 width, const u32 height, StRendererVulkanSwapchain *pSwapchain);

/**
 * Destroys a vulkan swapchain.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param pSwapchain Pointer to a swapchain to destroy.
 */
void stRendererVulkanSwapchainDestroy(StRendererVulkanBackend *pBackend, StRendererVulkanSwapchain *pSwapchain);

/**
 * Recreates a vulkan swapchain.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param width New swapchain width.
 * @param height New swapchain height.
 */
bool stRendererVulkanSwapchainRecreate(StRendererVulkanBackend *pBackend, const u32 width, const u32 height, StRendererVulkanSwapchain *pSwapchain);

/**
 * Updates the internal vulkan swapchain image index.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param timeout_ns Timeout for acquiring the next frame in nanoseconds.
 * @param fence vulkan fence used to prevent this swapchain image from being accessed asynchronously.
 * @return True if successful.
 * @note This must be called after each frame.
 */
bool stRendererVulkanSwapchainAcquireNextImageIndex(StRendererVulkanContext *pContext, const u64 timeout_ns, VkFence fence);

/**
 * Presents an image from the swapchain.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param render_complete_semaphore The vulkan semaphore that will be signaled when the vulkan image is ready to be presented.
 * @param present_image_index The index of the swapchain image to present to the screen.
 * @note stRendererVulkanSwapchainAcquireNextImageIndex() must be called before this.
 */
bool stRendererVulkanSwapchainPresent(StRendererVulkanContext *pContext, VkSemaphore render_complete_semaphore, const u32 present_image_index);
