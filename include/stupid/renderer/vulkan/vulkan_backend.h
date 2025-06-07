#pragma once

#include "stupid/renderer/vulkan/vulkan_types.h"

/**
 * @brief Initializes the Vulkan rendering backend.
 * @note Dont create more than one Vulkan backend.
 * @return A new Vulkan backend if successful, NULL otherwise.
 */
StRendererVulkanBackend *stRendererVulkanBackendInit(void);

/**
 * kills the Vulkan rendering backend
 * @param pBackend The Vulkan backend to destroy.
 * @return True if successful.
 */
bool stRendererVulkanBackendShutdown(StRendererVulkanBackend *pBackend);

