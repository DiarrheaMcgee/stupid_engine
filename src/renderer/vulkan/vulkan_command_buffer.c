#include "stupid/renderer/vulkan/vulkan_command_buffer.h"
#include "stupid/renderer/vulkan/vulkan_types.h"
#include "stupid/renderer/vulkan/vulkan_utils.h"
#include <vulkan/vulkan_core.h>

void stRendererVulkanCommandBufferAllocate(const StRendererVulkanContext *pContext, const VkCommandPool pool, const bool is_primary, StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);
	STUPID_NC(pContext->pBackend->device.logical_device);
	STUPID_NC(pCommandBuffer);

	VkCommandBufferAllocateInfo allocate_info = {0};
	allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool        = pool;
	allocate_info.commandBufferCount = 1;
	allocate_info.pNext              = NULL;

	if (is_primary)
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	else
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	pCommandBuffer->state	    = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED;

	// wait to make sure there are no active graphics operations
	vkDeviceWaitIdle(pContext->pBackend->device.logical_device);

	VK_CHECK(vkAllocateCommandBuffers(pContext->pBackend->device.logical_device, &allocate_info, &pCommandBuffer->handle));
	pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_READY;

	STUPID_LOG_TRACE("created vulkan graphics command buffer %p", &pCommandBuffer->handle);
}

void stRendererVulkanCommandBufferFree(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pContext);
	STUPID_NC(pCommandBuffer);
	vkDeviceWaitIdle(pContext->pBackend->device.logical_device);
	vkFreeCommandBuffers(pContext->pBackend->device.logical_device, pool, 1, &pCommandBuffer->handle);
	STUPID_LOG_TRACE("destroyed command buffer %p in pool %p", &pCommandBuffer->handle, &pool);
	pCommandBuffer->handle = NULL;
	pCommandBuffer->state  = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void stRendererVulkanCommandBufferBegin(const bool is_single_use, const bool is_renderpass_continue, const bool is_simultaneous_use, StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pCommandBuffer);
	STUPID_ASSERT(pCommandBuffer->state == ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_READY, "invalid command buffer state");

	VkCommandBufferBeginInfo begin = {0};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = 0;

	// specifies that the command buffer will be reset each time after submitting commands
	if (is_single_use)
		begin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// specifies that the command buffer is used entirely in between starting and finishing a renderpass
	if (is_renderpass_continue)
		begin.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

	// specifies that the command buffer can be used across queues
	if (is_simultaneous_use)
		begin.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VK_CHECK(vkBeginCommandBuffer(pCommandBuffer->handle, &begin));

	pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING;
}

void stRendererVulkanCommandBufferEnd(StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pCommandBuffer);
	STUPID_ASSERT(pCommandBuffer->state == ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING, "invalid command buffer state");
	VK_CHECK(vkEndCommandBuffer(pCommandBuffer->handle));
	pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_SUBMITTED;
}

void stRendererVulkanCommandBufferReset(StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pCommandBuffer);
	vkResetCommandBuffer(pCommandBuffer->handle, 0);
	pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_READY;
}

void stRendererVulkanCommandBufferBeginTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pContext);
	STUPID_NC(pCommandBuffer);
	stRendererVulkanCommandBufferAllocate(pContext, pool, true, pCommandBuffer);
	stRendererVulkanCommandBufferBegin(true, false, false, pCommandBuffer);
}

void stRendererVulkanCommandBufferEndTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, const VkQueue queue, StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pContext);
	STUPID_NC(pCommandBuffer);
	stRendererVulkanCommandBufferEnd(pCommandBuffer);

	VkSubmitInfo submit = {0};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers	  = &pCommandBuffer->handle;

	VK_CHECK(vkQueueSubmit(queue, 1, &submit, NULL));
	VK_CHECK(vkQueueWaitIdle(queue));

	stRendererVulkanCommandBufferFree(pContext, pool, pCommandBuffer);
}
