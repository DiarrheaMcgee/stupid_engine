#pragma once

#include "stupid/common.h"
#include "stupid/render/vulkan/vulkan_types.h"

/// TOOD: figure out a comment for this
i32 stRendererVulkanMemoryGetIndex(const StRendererVulkanBackend *pBackend, const u32 type_filter, const u32 property_flags);

/**
 * Allocates a VRAM buffer with the specified properties.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param size Number of bytes to allocate.
 * @param flags Things the buffer will be used for.
 * @param property Special properties for the buffer.
 * @param pBuffer Output buffer.
 */
void stRendererVulkanMemoryAllocate(StRendererVulkanBackend *pBackend, const usize size, const VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits property, StRendererVulkanBuffer *pBuffer);

/**
 * Maps a VRAM buffer to memory.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param map Pointer to store the location of the mapped memory in.
 * @param pBuffer Buffer to map to memory.
 */
void stRendererVulkanMemoryMap(StRendererVulkanBackend *pBackend, void **map, StRendererVulkanBuffer *pBuffer);

/**
 * Unmaps a VRAM buffer from memory.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param pBuffer Buffer to unmap from memory.
 */
void stRendererVulkanMemoryUnmap(StRendererVulkanBackend *pBackend, StRendererVulkanBuffer *pBuffer);

/**
 * Deallocates a VRAM buffer.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param pBuffer Buffer to deallocate.
 */
void stRendererVulkanMemoryDeallocate(StRendererVulkanBackend *pBackend, StRendererVulkanBuffer *pBuffer);

/**
 * Copies one VRAM buffer to another.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param size Number of bytes to copy.
 * @param pDestBuffer Destination buffer.
 * @param pSrcBuffer Source buffer.
 * @param dest_offset Destination offset.
 * @param src_offset Source offset.
 */
void stRendererVulkanMemoryCopyBuffer(StRendererVulkanContext *pContext, const usize size, StRendererVulkanBuffer *pDestBuffer, StRendererVulkanBuffer *pSrcBuffer, const usize dest_offset, const usize src_offset);

