#pragma once

#include "stupid/render/vulkan/vulkan_types.h"

/**
 * Creates a vulkan device.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param pRequirements Requirements for the device.
 * @return A pointer to a device.
 */
bool stRendererVulkanCreateDevice(VkInstance instance, VkAllocationCallbacks *pAllocator, StRendererVulkanDeviceRequirements *pRequirements, StRendererVulkanDevice *pDevice);

/**
 * Checks if a device has swapchain support.
 * @param pSwapchainSupport Pointer to an output StRendererVulkanSwapchainSupport struct.
 * @param physical_device GPU to check.
 * @param surface vulkan surface.
 * @return True if successful.
 */
bool stRendererVulkanDeviceQuerySwapchainSupport(StRendererVulkanSwapchainSupport *pSwapchainSupport, VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/**
 * Gets the depth format of the GPU.
 * @param pDevice Device to check.
 * @return True if successful.
 */
bool stRendererVulkanDeviceGetDepthFormat(StRendererVulkanDevice *pDevice);

/**
 * Destroys a device.
 * @param pBackend Pointer to a vulkan renderer backend.
 * @param pDevice Pointer to a device to destroy.
 */
void stRendererVulkanDeviceDestroy(VkAllocationCallbacks *pAllocator, StRendererVulkanDevice *pDevice);

