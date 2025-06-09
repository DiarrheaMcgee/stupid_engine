#pragma once

#include "stupid/renderer/vulkan/vulkan_types.h"

/**
 * Creates a vulkan fence.
 * @param pBackend Pointer to a vulkan backend created with stRendererVulkanBackendInit.
 * @param create_signaled Whether the fence should be pre signalled.
 * @param pFence Pointer to the output vulkan fence.
 */
void stRendererVulkanFenceCreate(const StRendererVulkanBackend *pBackend, const bool create_signaled, VkFence *pFence);

/**
 * destroys a vulkan fence synchronization object
 * @param pBackend Pointer to a vulkan backend created with stRendererVulkanBackendInit.
 * @param pFence Pointer to the vulkan fence to be destroyed.
 */
void stRendererVulkanFenceDestroy(const StRendererVulkanBackend *pBackend, VkFence pFence);

/**
 * Waits for a vulkan fence to be signaled.
 * @param pBackend Pointer to a vulkan backend created with stRendererVulkanBackendInit.
 * @param pFence Pointer to a vulkan fence to wait for.
 * @return True if the vulkan fence was signalled 
 */
bool stRendererVulkanFenceWait(const StRendererVulkanBackend *pBackend, const VkFence *pFence);

/**
 * Resets a vulkan fence.
 * @param pBackend Pointer to a vulkan backend created with stRendererVulkanBackendInit.
 * @param pFence Pointer to a vulkan fence to reset.
 */
void stRendererVulkanFenceReset(const StRendererVulkanBackend *pBackend, const VkFence *pFence);

