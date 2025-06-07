#include "stupid/renderer/vulkan/vulkan_fence.h"
#include "stupid/renderer/vulkan/vulkan_utils.h"

#include "stupid/logger.h"

void stRendererVulkanFenceCreate(const StRendererVulkanBackend *pBackend, const bool create_signaled, VkFence *pFence)
{
        STUPID_NC(pFence);

        VkFenceCreateInfo fence_info = {0};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.pNext = pBackend->pAllocator;

        if (create_signaled)
                fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        else
                fence_info.flags = 0;

        VK_CHECK(vkCreateFence(pBackend->device.logical_device, &fence_info, pBackend->pAllocator, pFence));
}

void stRendererVulkanFenceDestroy(const StRendererVulkanBackend *pBackend, VkFence pFence)
{
        STUPID_NC(pBackend);
        STUPID_NC(pBackend->device.logical_device);
        STUPID_NC(pFence);
        vkDestroyFence(pBackend->device.logical_device, pFence, pBackend->pAllocator);
}

bool stRendererVulkanFenceWait(const StRendererVulkanBackend *pBackend, const VkFence *pFence)
{
        STUPID_NC(pFence);

        const VkResult result = vkWaitForFences(pBackend->device.logical_device, 1, pFence, VK_TRUE, STUPID_SEC_TO_NS(1));

        if (result == VK_SUCCESS) return true;
        else {
                STUPID_LOG_ERROR("stRendererVulkanFenceWait(): %s", stRendererVulkanResultStr(result, true));
                return false;
        }
}

void stRendererVulkanFenceReset(const StRendererVulkanBackend *pBackend, const VkFence *pFence)
{
        VK_CHECK(vkResetFences(pBackend->device.logical_device, 1, pFence));
}
