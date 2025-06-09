#pragma once

#include "stupid/renderer/vulkan/vulkan_types.h"

/**
 * @brief Initializes the vulkan rendering backend.
 * @note Dont create more than one vulkan backend.
 * @return A new vulkan backend if successful, NULL otherwise.
 */
StRendererVulkanBackend *stRendererVulkanBackendInit(void);

/**
 * kills the vulkan rendering backend
 * @param pBackend The vulkan backend to destroy.
 * @return True if successful.
 */
bool stRendererVulkanBackendShutdown(StRendererVulkanBackend *pBackend);

