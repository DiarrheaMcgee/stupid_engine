#pragma once

#include "stupid/renderer/vulkan/vulkan_types.h"

/**
 * Creates a Vulkan device.
 * @param pBackend Pointer to a Vulkan renderer backend.
 * @param pRequirements Requirements for the device.
 * @return A pointer to a device.
 */
//StRendererVulkanDevice *stRendererVulkanCreateDevice(StRendererVulkanBackend *pBackend, StRendererVulkanDeviceRequirements *pRequirements);
bool stRendererVulkanCreateDevice(VkInstance instance, VkAllocationCallbacks *pAllocator, StRendererVulkanDeviceRequirements *pRequirements, StRendererVulkanDevice *pDevice);

/**
 * Checks if a device has swapchain support.
 * @param pSwapchainSupport Pointer to an output StRendererVulkanSwapchainSupport struct.
 * @param physical_device GPU to check.
 * @param surface StRendererVulkan surface.
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
 * @param pBackend Pointer to a Vulkan renderer backend.
 * @param pDevice Pointer to a device to destroy.
 */
//void stRendererVulkanDeviceDestroy(const StRendererVulkanBackend *pBackend, StRendererVulkanDevice *pDevice);
void stRendererVulkanDeviceDestroy(StRendererVulkanDevice *pDevice, VkAllocationCallbacks *pAllocator);

