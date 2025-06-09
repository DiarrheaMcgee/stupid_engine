#pragma once

#include "stupid/renderer/vulkan/vulkan_types.h"

/**
 * Allocates a vulkan command buffer.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param pool A vulkan command pool handle (basically a pool of vulkan command buffers).
 * @param is_primary Whether this is a primary vulkan command buffer.
 * @param pCommandBuffer Pointer to the output StRendererVulkanCommandBuffer.
 */
void stRendererVulkanCommandBufferAllocate(const StRendererVulkanContext *pContext, const VkCommandPool pool, const bool is_primary, StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * Frees a vulkan command buffer.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param pool A vulkan command pool handle.
 * @param pCommandBuffer Pointer to the StRendererVulkanCommandBuffer to free.
 */
void stRendererVulkanCommandBufferFree(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * Makes a vulkan command buffer start recording comands.
 * @param is_single_use Whether this vulkan command buffer should only be submitted once.
 * @param is_renderpass_continue Whether this vulkan command buffer is entirely inside of a renderpass.
 * @param is_simultaneous_use Whether this vulkan command buffer can be used by multiple queues.
 * @param pCommandBuffer Pointer to the StRendererVulkanCommandBuffer to begin recording with.
 */
void stRendererVulkanCommandBufferBegin(const bool is_single_use, const bool is_renderpass_continue, const bool is_simultaneous_use, StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * Makes a vulkan command buffer finish recording.
 * @param pCommandBuffer Pointer to the StRendererVulkanCommandBuffer to stop recording on.
 */
void stRendererVulkanCommandBufferEnd(StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * Resets a vulkan command buffer.
 * @param pCommandBuffer pointer to a command buffer struct
 */
void stRendererVulkanCommandBufferReset(StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * Creates a temporary vulkan command buffer that can only be used once.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param pool A vulkan command pool handle (basically a pool of vulkan command buffers)
 * @param pCommandBuffer Pointer to the output StRendererVulkanCommandBuffer.
 */
void stRendererVulkanCommandBufferBeginTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer);

/**
 * Submits and destroys a temporary vulkan command buffer.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param pool A vulkan command pool handle.
 * @param queue Vulkan queue to submit the command buffer to.
 * @param pCommandBuffer Pointer to the temporary StRendererVulkanCommandBuffer to end.
 */
void stRendererVulkanCommandBufferEndTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, const VkQueue queue, StRendererVulkanCommandBuffer *pCommandBuffer);

