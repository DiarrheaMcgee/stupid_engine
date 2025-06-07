#pragma once

#include "stupid/renderer/vulkan/vulkan_types.h"

/**
 * Creates a Vulkan fence.
 * @param pBackend Pointer to a Vulkan backend created with stRendererVulkanBackendInit.
 * @param create_signaled Whether the fence should be pre signalled.
 * @param pFence Pointer to the output Vulkan fence.
 */
void stRendererVulkanFenceCreate(const StRendererVulkanBackend *pBackend, const bool create_signaled, VkFence *pFence);

/**
 * destroys a Vulkan fence synchronization object
 * @param pBackend Pointer to a Vulkan backend created with stRendererVulkanBackendInit.
 * @param pFence Pointer to the Vulkan fence to be destroyed.
 */
void stRendererVulkanFenceDestroy(const StRendererVulkanBackend *pBackend, VkFence pFence);

/**
 * Waits for a Vulkan fence to be signaled.
 * @param pBackend Pointer to a Vulkan backend created with stRendererVulkanBackendInit.
 * @param pFence Pointer to a Vulkan fence to wait for.
 * @return True if the Vulkan fence was signalled 
 */
bool stRendererVulkanFenceWait(const StRendererVulkanBackend *pBackend, const VkFence *pFence);

/**
 * Resets a Vulkan fence.
 * @param pBackend Pointer to a Vulkan backend created with stRendererVulkanBackendInit.
 * @param pFence Pointer to a Vulkan fence to reset.
 */
void stRendererVulkanFenceReset(const StRendererVulkanBackend *pBackend, const VkFence *pFence);

