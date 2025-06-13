#include "stupid/render/vulkan/vulkan_memory.h"
#include "stupid/render/vulkan/vulkan_command_buffer.h"

#include "stupid/memory.h"
#include "stupid/assert.h"

i32 stRendererVulkanMemoryGetIndex(const StRendererVulkanBackend *pBackend, const u32 type_filter, const u32 property_flags)
{
        VkPhysicalDeviceMemoryProperties memory_properties = {0};
        vkGetPhysicalDeviceMemoryProperties(pBackend->device.physical_device, &memory_properties);

	// checks for an area in vram that satisfies the requirements
        for (int i = 0; i < memory_properties.memoryTypeCount; i++) {
                if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
                        return i;
        }

        STUPID_LOG_ERROR("unable to find a memory type good enough for my high standards");
        return -1;
}

void stRendererVulkanMemoryAllocate(StRendererVulkanBackend *pBackend, const usize size, const VkBufferUsageFlagBits flags, VkMemoryPropertyFlagBits property, StRendererVulkanBuffer *pBuffer)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pBuffer);

	stMemset(pBuffer, 0, sizeof(StRendererVulkanBuffer));

	VkBufferCreateInfo buffer_info = {0};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = flags;
	buffer_info.size  = size;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkCreateBuffer(pBackend->device.logical_device, &buffer_info, pBackend->pAllocator, &pBuffer->handle);

        VkMemoryRequirements memory_requirements = {0};
	vkGetBufferMemoryRequirements(pBackend->device.logical_device, pBuffer->handle, &memory_requirements);

	VkMemoryAllocateFlagsInfo flag_info = {0};
	flag_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	flag_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

        VkMemoryAllocateInfo allocate_info = {0};
	allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocate_info.allocationSize = memory_requirements.size;
	allocate_info.memoryTypeIndex = stRendererVulkanMemoryGetIndex(pBackend, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
	allocate_info.pNext = &flag_info;
	pBuffer->size = memory_requirements.size;

	vkAllocateMemory(pBackend->device.logical_device, &allocate_info, pBackend->pAllocator, &pBuffer->memory);
	vkBindBufferMemory(pBackend->device.logical_device, pBuffer->handle, pBuffer->memory, 0);

	if (flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
		VkBufferDeviceAddressInfo bda_info = {0};
		bda_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bda_info.buffer = pBuffer->handle;
		pBuffer->address = vkGetBufferDeviceAddress(pBackend->device.logical_device, &bda_info);
	}
}

void stRendererVulkanMemoryMap(StRendererVulkanBackend *pBackend, void **map, StRendererVulkanBuffer *pBuffer)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pBuffer);
	STUPID_NC(map);

	vkMapMemory(pBackend->device.logical_device, pBuffer->memory, 0, pBuffer->size, 0, map);
}

void stRendererVulkanMemoryMapImage(StRendererVulkanBackend *pBackend, StRendererVulkanImage *pImage, StRendererVulkanBuffer *pBuffer)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pBuffer);
	STUPID_NC(pImage);
}

void stRendererVulkanMemoryUnmap(StRendererVulkanBackend *pBackend, StRendererVulkanBuffer *pBuffer)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pBuffer);

	VkMappedMemoryRange range = {0};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = pBuffer->memory;
	range.size = pBuffer->size;
	vkFlushMappedMemoryRanges(pBackend->device.logical_device, 1, &range);

	vkUnmapMemory(pBackend->device.logical_device, pBuffer->memory);
}

void stRendererVulkanMemoryDeallocate(StRendererVulkanBackend *pBackend, StRendererVulkanBuffer *pBuffer)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);
	STUPID_NC(pBuffer);

	vkDeviceWaitIdle(pBackend->device.logical_device);
	vkFreeMemory(pBackend->device.logical_device, pBuffer->memory, pBackend->pAllocator);
	vkDestroyBuffer(pBackend->device.logical_device, pBuffer->handle, pBackend->pAllocator);
	pBuffer->memory = VK_NULL_HANDLE;
	pBuffer->handle = VK_NULL_HANDLE;
	pBuffer->size   = 0;
}

void stRendererVulkanMemoryCopyBuffer(StRendererVulkanContext *pContext, const usize size, StRendererVulkanBuffer *pDestBuffer, StRendererVulkanBuffer *pSrcBuffer, const usize dest_offset, const usize src_offset)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);
	STUPID_NC(pContext->pBackend->device.logical_device);
	STUPID_NC(pDestBuffer);
	STUPID_NC(pSrcBuffer);
	STUPID_ASSERT(size > 0, "invalid size");
	STUPID_ASSERT(dest_offset + size <= pDestBuffer->size, "offset + size out of destination buffer bounds");
	STUPID_ASSERT(src_offset + size <= pSrcBuffer->size, "offset + size out of source buffer bounds");

	VkBufferCopy copy = {0};
	copy.size = size;
	copy.srcOffset = src_offset;
	copy.dstOffset = dest_offset;

	StRendererVulkanCommandBuffer cmd = {0};
	stRendererVulkanCommandBufferBeginTemporary(pContext, pContext->pBackend->device.graphics_command_pool, &cmd);
	vkCmdCopyBuffer(cmd.handle, pSrcBuffer->handle, pDestBuffer->handle, 1, &copy);
	stRendererVulkanCommandBufferEndTemporary(pContext, pContext->pBackend->device.graphics_command_pool, pContext->pBackend->device.graphics_queue, &cmd);
}

