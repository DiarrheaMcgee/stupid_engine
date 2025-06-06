#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"
#include <vulkan/vulkan_core.h>

void stRendererVulkanCommandBufferAllocate(const StRendererVulkanContext *pContext, const VkCommandPool pool, const bool is_primary, StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);
	STUPID_NC(pContext->pBackend->device.logical_device);
	STUPID_NC(pCommandBuffer);

        // information about allocating memory for the Vulkan command buffer
        VkCommandBufferAllocateInfo allocate_info = {0};

        // must be VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
        allocate_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        // Vulkan command pool handle (basically a collection of Vulkan command buffers)
        allocate_info.commandPool = pool;

        if (is_primary)
                allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        else
                allocate_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

        allocate_info.commandBufferCount = 1;
        allocate_info.pNext              = NULL;
        pCommandBuffer->state       = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED;

        // wait to make sure there are no active graphics operations
        vkDeviceWaitIdle(pContext->pBackend->device.logical_device);

        // attempt to allocate the Vulkan command buffer
        VK_CHECK(vkAllocateCommandBuffers(pContext->pBackend->device.logical_device, &allocate_info, &pCommandBuffer->handle));

        // set the state of the Vulkan command buffer to ready
        pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_READY;

        STUPID_LOG_TRACE("created Vulkan graphics command buffer %p", &pCommandBuffer->handle);
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

void stRendererVulkanCommandBufferBegin(StRendererVulkanCommandBuffer *pCommandBuffer, const bool is_single_use, const bool is_renderpass_continue, const bool is_simultaneous_use)
{
	STUPID_NC(pCommandBuffer);
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING_ENDED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_SUBMITTED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING, "invalid command buffer state");

        // information about starting the Vulkan command buffer
        VkCommandBufferBeginInfo begin = {0};

        // must be VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        // enabled flags for starting the Vulkan command buffer
        begin.flags = 0;

        // specifies that each recording of this command buffer will only be submitted once
        // and will be reset and recorded again between each submission
        if (is_single_use)
                begin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        // specifies that this is a secondary command buffer entirely inside a render pass
        // (basically if this has nothing to do with subpasses)
        if (is_renderpass_continue)
                begin.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

        // specifies that this can be used across multiple queues
        if (is_simultaneous_use)
                begin.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // attempt to start the Vulkan command buffer recording phase
        VK_CHECK(vkBeginCommandBuffer(pCommandBuffer->handle, &begin));

        pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING;
}

void stRendererVulkanCommandBufferEnd(StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pCommandBuffer);
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING_ENDED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_SUBMITTED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED, "invalid command buffer state");
        VK_CHECK(vkEndCommandBuffer(pCommandBuffer->handle));
        pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void stRendererVulkanCommandBufferUpdateSubmitted(StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pCommandBuffer);
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_SUBMITTED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING, "invalid command buffer state");
        // TODO: fill
        pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_SUBMITTED;
}

void stRendererVulkanCommandBufferReset(StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pCommandBuffer);
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING_ENDED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_NOT_ALLOCATED, "invalid command buffer state");
	STUPID_ASSERT(pCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING, "invalid command buffer state");
        // TODO: fill
        pCommandBuffer->state = ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_READY;
}

void stRendererVulkanCommandBufferBeginTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer)
{
	STUPID_NC(pContext);
	STUPID_NC(pCommandBuffer);
        stRendererVulkanCommandBufferAllocate(pContext, pool, true, pCommandBuffer);
        stRendererVulkanCommandBufferBegin(pCommandBuffer, true, false, false);
}

void stRendererVulkanCommandBufferEndTemporary(const StRendererVulkanContext *pContext, const VkCommandPool pool, StRendererVulkanCommandBuffer *pCommandBuffer, const VkQueue queue)
{
	STUPID_NC(pContext);
	STUPID_NC(pCommandBuffer);
        stRendererVulkanCommandBufferEnd(pCommandBuffer);

        VkSubmitInfo submit = {0};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers    = &pCommandBuffer->handle;

        VK_CHECK(vkQueueSubmit(queue, 1, &submit, NULL));
        VK_CHECK(vkQueueWaitIdle(queue));

        stRendererVulkanCommandBufferFree(pContext, pool, pCommandBuffer);
}
