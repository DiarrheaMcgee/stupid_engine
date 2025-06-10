#pragma once

#include "stupid/common.h"
#include "stupid/render/vulkan/vulkan_types.h"

i32 stRendererVulkanMemoryGetIndex(const StRendererVulkanBackend *pBackend, const u32 type_filter, const u32 property_flags);

void stRendererVulkanMemoryAllocate(StRendererVulkanBackend *pBackend, const usize size, const VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits property, StRendererVulkanBuffer *pBuffer);

void stRendererVulkanMemoryMap(StRendererVulkanBackend *pBackend, void **map, StRendererVulkanBuffer *pBuffer);

void stRendererVulkanMemoryUnmap(StRendererVulkanBackend *pBackend, StRendererVulkanBuffer *pBuffer);

void stRendererVulkanMemoryDeallocate(StRendererVulkanBackend *pBackend, StRendererVulkanBuffer *pBuffer);

void stRendererVulkanMemoryCopyBuffer(StRendererVulkanContext *pContext, const usize size, StRendererVulkanBuffer *pDestBuffer, StRendererVulkanBuffer *pSrcBuffer, const usize dest_offset, const usize src_offset);

