#include "stupid/render/vulkan/vulkan_device.h"
#include "stupid/render/vulkan/vulkan_utils.h"

#include "stupid/common.h"
#include "stupid/window.h"
#include "stupid/logger.h"
#include "stupid/memory.h"

/// vulkan queue families.
/// @note Look it up.
typedef struct QueueFamilies {
	struct {
		bool graphics, present, transfer, compute;
		int count;
		int used_queue_count;
	} pQueues[4];
	int graphics_index, present_index, transfer_index, compute_index;
	int queue_count;
} QueueFamilies;

bool stRendererVulkanDeviceQuerySwapchainSupport(StRendererVulkanSwapchainSupport *pSwapchainSupport, VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
	stMemset(pSwapchainSupport, 0, sizeof(StRendererVulkanSwapchainSupport));
	if (!stRendererVulkanResultIsSuccess(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &pSwapchainSupport->capabilities))) return false;

	if (!stRendererVulkanResultIsSuccess(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &pSwapchainSupport->format_count, NULL))) return false;
	STUPID_ASSERT(pSwapchainSupport->format_count >= 1, "no surface formats are available so the driver is probably broken");
	if (!stRendererVulkanResultIsSuccess(vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &pSwapchainSupport->format_count, pSwapchainSupport->pFormats))) return false;

	if (!stRendererVulkanResultIsSuccess(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &pSwapchainSupport->present_mode_count, NULL))) return false;
	STUPID_ASSERT(pSwapchainSupport->present_mode_count >= 1, "no present modes are available so the driver is probably broken");
	if (!stRendererVulkanResultIsSuccess(vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &pSwapchainSupport->present_mode_count, pSwapchainSupport->pPresentModes))) return false;

	return true;
}

bool stRendererVulkanCheckSwapchainRequirements(StRendererVulkanSwapchainSupport *pCapabilities, StRendererVulkanSwapchainSupport *pRequirements)
{
	STUPID_NC(pCapabilities);
	STUPID_NC(pRequirements);

#define CHECK_G(member) if (pRequirements->member != 0 && pRequirements->member >= pCapabilities->member) return false
#define CHECK_L(member) if (pRequirements->member != 0 && pRequirements->member <= pCapabilities->member) return false

	// check if any of the requirement counts are higher than the capability counts
	CHECK_G(format_count);
	CHECK_G(present_mode_count);
	CHECK_L(capabilities.minImageCount);
	CHECK_G(capabilities.maxImageCount);
	CHECK_G(capabilities.maxImageArrayLayers);
	CHECK_L(capabilities.minImageExtent.width);
	CHECK_L(capabilities.minImageExtent.height);
	CHECK_G(capabilities.maxImageExtent.width);
	CHECK_G(capabilities.maxImageExtent.height);

	// check for all the required usage flag bits
	for (int i = 0; i < sizeof(pRequirements->capabilities.supportedUsageFlags) * 8; i++)
		if ((pRequirements->capabilities.supportedUsageFlags & (1 << i)) != 0 && (pCapabilities->capabilities.supportedUsageFlags & (1 << i)) == 0) return false;

	// check for all the required composite alpha bits
	for (int i = 0; i < sizeof(pRequirements->capabilities.supportedCompositeAlpha) * 8; i++)
		if ((pRequirements->capabilities.supportedCompositeAlpha & (1 << i)) != 0 && (pCapabilities->capabilities.supportedCompositeAlpha & (1 << i)) == 0) return false;

	// check for all the required surface formats
	for (int i = 0; i < pRequirements->format_count; i++)
		if (stFind(pCapabilities->pFormats, &pRequirements->pFormats[i], sizeof(VkFormat), STUPID_MIN(stMemLength(pCapabilities->pFormats), stMemLength(pRequirements->pFormats))) == -1)
			return false;

	// check for all the required present modes
	for (int i = 0; i < pRequirements->present_mode_count; i++)
		if (stFind(pCapabilities->pPresentModes, &pRequirements->pPresentModes[i], sizeof(VkPresentModeKHR), sizeof(pRequirements->pPresentModes) / sizeof(VkPresentModeKHR)) == -1)
			return false;

	return true;
}

/**
 * Attempt to get the optimal vulkan queue family indices from a VkPhysicalDevice.
 * @param physical_device GPU to get queue family indices from.
 * @param surface If surface is not VK_NULL_HANDLE, then it will be used to query present support.
 * Otherwise, present support will not be checked.
 */
static QueueFamilies getQueueFamilies(VkInstance instance, VkPhysicalDevice physical_device)
{
	// output queue families
	QueueFamilies queue_families = {0};
	queue_families.graphics_index = -1;
	queue_families.present_index = -1;
	queue_families.transfer_index = -1;
	queue_families.compute_index = -1;

	// somewhat arbitrary number of VkQueueFamilyProperties objects to statically allocate.
#define QUEUE_FAMILY_PROPERTY_SIZE 12

	// number of available queue families
	u32 count = 0;

	// queue family array (i dont want another heap allocation here so this should be enough)
	VkQueueFamilyProperties pProperties[QUEUE_FAMILY_PROPERTY_SIZE];

	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, NULL);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, pProperties);

	for (int i = 0; i < STUPID_MIN(count, QUEUE_FAMILY_PROPERTY_SIZE); i++) {
		if (pProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queue_families.pQueues[i].graphics = true;
			queue_families.pQueues[i].count++;
		}
		if (pProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			queue_families.pQueues[i].transfer = true;
			queue_families.pQueues[i].count++;
		}
		if (pProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			queue_families.pQueues[i].compute = true;
			queue_families.pQueues[i].count++;
		}
		if (stWindowGetVulkanPresentationSupport(instance, physical_device, i)) {
			queue_families.pQueues[i].present = true;
			queue_families.pQueues[i].count++;
		}

		// change the current graphics/present/transfer/compute queue index to this one if its better (or if it hasent already been set)
		if (queue_families.pQueues[i].graphics && (queue_families.graphics_index == -1 || (queue_families.pQueues[queue_families.graphics_index].count > queue_families.pQueues[i].count)))
			queue_families.graphics_index = i;

		if (queue_families.pQueues[i].present && (queue_families.present_index == -1 || (queue_families.pQueues[queue_families.present_index].count > queue_families.pQueues[i].count)))
			queue_families.present_index = i;

		if (queue_families.pQueues[i].transfer && (queue_families.transfer_index == -1 || (queue_families.pQueues[queue_families.transfer_index].count > queue_families.pQueues[i].count)))
			queue_families.transfer_index = i;

		if (queue_families.pQueues[i].compute && (queue_families.compute_index == -1 || (queue_families.pQueues[queue_families.compute_index].count > queue_families.pQueues[i].count)))
			queue_families.compute_index = i;

		STUPID_LOG_DEBUG("%d: %d %d %d %d", i, queue_families.pQueues[i].graphics, queue_families.pQueues[i].present, queue_families.pQueues[i].transfer, queue_families.pQueues[i].compute);
	}

	i32 indices[4] = {
		queue_families.graphics_index,
		queue_families.present_index,
		queue_families.transfer_index,
		queue_families.compute_index
	};

	// count the unique indices
	for (int i = 0; i < 4; i++) {		
		bool unique = true;
		for (int j = 0; j < 4; j++) {
			if (i == j) continue;
			if (indices[i] == indices[j]) unique = false;
		}
		if (unique) queue_families.queue_count++;
	}
	
	queue_families.pQueues[queue_families.graphics_index].used_queue_count += 1;
	queue_families.pQueues[queue_families.present_index].used_queue_count += 1;
	queue_families.pQueues[queue_families.transfer_index].used_queue_count += 1;
	queue_families.pQueues[queue_families.compute_index].used_queue_count += 1;

	// increment it again since it starts at 0
	queue_families.queue_count++;

	STUPID_LOG_DEBUG("%d: %d %d %d %d", queue_families.queue_count, queue_families.graphics_index, queue_families.present_index, queue_families.transfer_index, queue_families.compute_index);

	return queue_families;
}

bool stRendererVulkanCreateDevice(VkInstance instance, VkAllocationCallbacks *pAllocator, StRendererVulkanDeviceRequirements *pRequirements, StRendererVulkanDevice *pDevice)
{
	STUPID_NC(instance);
	STUPID_NC(pDevice);

	u32 device_count = 0;

	// acquire the list of physical gpus
	vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	VkPhysicalDevice *pDevices = stMemAllocNL(VkPhysicalDevice, device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, pDevices);

	// vulkan physical device handle
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;

	// window used to check for surface support
	StWindow *pUtilityWindow = NULL;

	// surface associated with the utility window
	VkSurfaceKHR utility_surface = VK_NULL_HANDLE;

	// initialize the utility window if surface support is required
	if (pRequirements->queue.present) {
		pUtilityWindow = stWindowCreateInvisible();
		VK_CHECK(stWindowCreateVulkanSurface(pUtilityWindow, instance, &utility_surface));
	}

	QueueFamilies queue_families = {0};

	bool found = false;

	for (int i = 0; i < device_count; i++) {
		physical_device = pDevices[i];
		QueueFamilies tmp = getQueueFamilies(instance, physical_device);
		if (pRequirements->queue.graphics && (!tmp.pQueues[0].graphics && !tmp.pQueues[1].graphics && !tmp.pQueues[2].graphics && !tmp.pQueues[3].graphics))
			continue;
		if (pRequirements->queue.present && (!tmp.pQueues[0].present && !tmp.pQueues[1].present && !tmp.pQueues[2].present && !tmp.pQueues[3].present))
			continue;
		if (pRequirements->queue.transfer && (!tmp.pQueues[0].transfer && !tmp.pQueues[1].transfer && !tmp.pQueues[2].transfer && !tmp.pQueues[3].transfer))
			continue;
		if (pRequirements->queue.compute && (!tmp.pQueues[0].compute && !tmp.pQueues[1].compute && !tmp.pQueues[2].compute && !tmp.pQueues[3].compute))
			continue;

		u32 ext_count = 0;
		vkEnumerateDeviceExtensionProperties(physical_device, NULL, &ext_count, NULL);
		VkExtensionProperties *pExtensions = stMemAllocNL(VkExtensionProperties, ext_count);
		vkEnumerateDeviceExtensionProperties(physical_device, NULL, &ext_count, pExtensions);

		// make sure all the required extensions exist
		bool found_extension = false;
		for (int j = 0; j < pRequirements->extension_count; j++) {
			for (int k = j; k < ext_count; k++) {
				if (stStrneq(pRequirements->extensions[j], pExtensions[k].extensionName, 128)) {
					found_extension = true;
					break;
				}
			}

			if (!found_extension) {
				STUPID_LOG_DEBUG("gpu %d is lacking extension '%s'", j, pRequirements->extensions[j]);
				break;
			}
		}
		stMemDeallocNL(pExtensions);


		// check the surface support
		if (pRequirements->queue.present) {
			if (!stRendererVulkanDeviceQuerySwapchainSupport(&pDevice->swapchain_support, physical_device, utility_surface))
				continue;
			if (!stRendererVulkanCheckSwapchainRequirements(&pDevice->swapchain_support, &pRequirements->swapchain_capabilities))
				continue;
		}

		found = true;
		queue_families = tmp;
		break;
	}

	stMemDeallocNL(pDevices);

	if (pRequirements->queue.present) {
		vkDestroySurfaceKHR(instance, utility_surface, pAllocator);
		stWindowDestroy(pUtilityWindow);
	}

	if (!found)
		return NULL;

	VkPhysicalDeviceBufferDeviceAddressFeatures bda_features = {0};
	bda_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
	bda_features.bufferDeviceAddress = VK_TRUE;

	VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {0};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;
	dynamic_rendering_features.pNext = &bda_features;

	VkDeviceQueueCreateInfo pQueueInfo[4];
	stMemset(pQueueInfo, 0, sizeof(VkDeviceQueueCreateInfo) * queue_families.queue_count);
	for (int i = 0; i < queue_families.queue_count; i++) {
		pQueueInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		pQueueInfo[i].queueCount = 1;
		float priority = 0.25f * (float)queue_families.pQueues[i].used_queue_count;
		pQueueInfo[i].pQueuePriorities = &priority;
		pQueueInfo[i].queueFamilyIndex = i;
	}

	VkDeviceCreateInfo device_info	    = {0};
	device_info.sType		    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext		    = &dynamic_rendering_features; // this is basically the only time pNext ever gets used (this enables the requested features)
	device_info.enabledExtensionCount   = pRequirements->extension_count;
	device_info.enabledLayerCount	    = pRequirements->layer_count;
	device_info.ppEnabledExtensionNames = pRequirements->extensions;
	device_info.ppEnabledLayerNames     = pRequirements->layers;
	device_info.queueCreateInfoCount    = queue_families.queue_count;
	device_info.pQueueCreateInfos	    = pQueueInfo;

	VK_CHECK(vkCreateDevice(physical_device, &device_info, pAllocator, &pDevice->logical_device));

	// pick the gpu queues (graphics and present will probably end up on the same queue)
	if (queue_families.graphics_index != -1) 
		vkGetDeviceQueue(pDevice->logical_device, queue_families.graphics_index, 0, &pDevice->graphics_queue);
	if (queue_families.present_index != -1)
		vkGetDeviceQueue(pDevice->logical_device, queue_families.present_index, 0, &pDevice->present_queue);
	if (queue_families.transfer_index != -1)
		vkGetDeviceQueue(pDevice->logical_device, queue_families.transfer_index, 0, &pDevice->transfer_queue);
	if (queue_families.compute_index != -1)
		vkGetDeviceQueue(pDevice->logical_device, queue_families.compute_index, 0, &pDevice->compute_queue);

	STUPID_LOG_DEBUG("vulkan device queue indices %d %d %d %d", pDevice->graphics_queue_index, pDevice->present_queue_index, pDevice->transfer_queue_index, pDevice->compute_queue_index);

	pDevice->physical_device = physical_device;
	vkGetPhysicalDeviceProperties(pDevice->physical_device, &pDevice->properties);
	vkGetPhysicalDeviceFeatures(pDevice->physical_device, &pDevice->features);

	// create a vulkan command pool for this device TODO: create one for each thread when multithreading is added to the renderer
	VkCommandPoolCreateInfo pool_info = {0};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = pDevice->graphics_queue_index;
	vkCreateCommandPool(pDevice->logical_device, &pool_info, pAllocator, &pDevice->graphics_command_pool);

	return true;
}

void stRendererVulkanDeviceDestroy(VkAllocationCallbacks *pAllocator, StRendererVulkanDevice *pDevice)
{
	STUPID_NC(pDevice);
	STUPID_NC(pDevice->logical_device);
	vkDestroyDevice(pDevice->logical_device, pAllocator);
}

bool stRendererVulkanDeviceGetDepthFormat(StRendererVulkanDevice *pDevice)
{
	STUPID_NC(pDevice);
	STUPID_NC(pDevice->logical_device);

	// list of possible vulkan depth formats (best to worst)
	const VkFormat candidates[3] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	const u8 candidate_count = sizeof(candidates) / sizeof(VkFormat);
	const u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	for (int i = 0; i < candidate_count; i++) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(pDevice->physical_device, candidates[i], &properties);

		// use the depth format if its found
		if ((properties.linearTilingFeatures & flags) == flags || (properties.optimalTilingFeatures & flags) == flags) {
			pDevice->depth_format = candidates[i];
			return true;
		}
	}

	return false;
}
