#pragma once

#include "vulkan_types.h"

/**
 * allocates a Vulkan command buffer
 * @param pContext Pointer to a Vulkan renderer instance.
 * @param pool a Vulkan command pool handle (basically a pool of Vulkan command buffers)
 * @param is_primary whether this is a primary Vulkan command buffer or not
 * @param pCommandBuffer pointer to the output command buffer struct
 */
void stRendererVulkanCommandBufferAllocate(const StRendererVulkanContext *pContext, const VkCommandPool pool, const bool is_primary, StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * attempt to free a Vulkan command buffer
 * @param pContext Pointer to a Vulkan renderer instance.
 * @param pool a Vulkan command pool handle (basically a pool of Vulkan command buffers)
 * @param pCommandBuffer a pointer to a command buffer struct
 */
void stRendererVulkanCommandBufferFree(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * attempts to start a Vulkan command buffers recording phase
 * @param pCommandBuffer pointer to a command buffer struct
 * @param is_single_use whether this Vulkan command buffer should only be submitted once or not
 * @param is_renderpass_continue whether this Vulkan command buffer is entirely inside of a renderpass or not
 * @param is_simultaneous_use whether this Vulkan command buffer can be used by multiple queues or not
 */
void stRendererVulkanCommandBufferBegin(StRendererVulkanCommandBuffer *pCommandBuffer, const bool is_single_use, const bool is_renderpass_continue, const bool is_simultaneous_use);

/**
 * attempts to end a Vulkan command buffers recording phase
 * @param pCommandBuffer pointer to a command buffer struct
 */
void stRendererVulkanCommandBufferEnd(StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * attempts to update all submitted Vulkan command buffers
 * @param pCommandBuffer pointer to a command buffer struct
 */
void stRendererVulkanCommandBufferUpdateSubmitted(StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * resets a Vulkan command buffer
 * @param pCommandBuffer pointer to a command buffer struct
 */
void stRendererVulkanCommandBufferReset(StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * creates a temporary Vulkan command buffer that is only used once
 * @param pContext Pointer to a Vulkan renderer instance.
 * @param pool a Vulkan command pool handle (basically a pool of Vulkan command buffers)
 * @param pCommandBuffer pointer to the output command buffer struct
 */
void stRendererVulkanCommandBufferBeginTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * kills a temporary Vulkan command
 * @param pContext Pointer to a Vulkan renderer instance.
 * @param pool a Vulkan command pool handle (basically a pool of Vulkan command buffers)
 * @param pCommandBuffer pointer to a command buffer struct
 * @param queue Vulkan queue handle
 */
void stRendererVulkanCommandBufferEndTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer, const VkQueue queue);

