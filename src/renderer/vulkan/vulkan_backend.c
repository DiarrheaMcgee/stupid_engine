#include "stupid/renderer/vulkan/vulkan_backend.h"
#include "stupid/renderer/vulkan/vulkan_device.h"
#include "stupid/renderer/vulkan/vulkan_utils.h"

#include "stupid/common.h"
#include "stupid/assert.h"
#include "stupid/logger.h"
#include "stupid/window.h"
#include "stupid/clock.h"
#include "stupid/memory.h"

#ifdef _DEBUG
/**
 * Vulkan validation error printer.
 * @param message_severity Message severity (error, warning, info, verbose).
 * @param message_types Unused.
 * @param pCallbackContext Struct containing the message.
 * @param pUserContext Unused.
 * @returns VK_FALSE
 * @note This function is a mess.
 * @todo Come up with something better.
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL stRendererVulkanDebugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
							  const VkDebugUtilsMessageTypeFlagsEXT message_types,
							  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackContext,
							  void *pUserContext)
{
	char *message = (char *)pCallbackContext->pMessage;

	const u64 len = stStrlen(message, 1024);

	int i = 0, j = 0;
	bool skipped_garbage = false;

	// insert linebreaks to make it more readable
	for (i = 0; i < len; i++) {
		// exit if a null terminator is reached
		if (message[i] == '\0')
			break;

		if (!skipped_garbage && message[i] == '|') {
			if (i != 0) {
				if (message[i - 1] == ' ') {
					message += i;
					skipped_garbage = true;
				}
			}
		}

		// this bowl of spaghetti extracts a link between parentheses and puts it on a separate line
		// check if the character is a ( or a )
		if (message[i] == '(' && message[i + 1] != ')') {

			// then check if the thing in there is a link
			if (message[i + 1] == 'h' && message[i + 2] == 't' && message[i + 3] == 't' && message[i + 4] == 'p') {

				// insert a linebreak in place of the (
				message[i] = '\n';

				// insert a linebreak at the other )
				for (int k = i; k < len; k++) {
					if (message[k] == ')') {
						message[k] = '\n';
						break;
					}
				}
			}
		}

		// replace the next space with a linebreak if there have been more than 150 characters
		else if (message[i] == ' ' && j >= 150) {
			message[i] = '\n';
			j          = 0;
		}

		// insert a linebreak if its the end of a sentence
		else if (message[i] == '.' && message[i + 1] == ' ') {
			message[i + 1] = '\n';
			j              = 0;
		}

		// reset j if there is already a linebreak
		else if (message[i] == '\n')
			j = 0;
		j++;
	}

	message[len] = '\0';

	switch (message_severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		STUPID_LOG_ERROR("%s", message);
		break;

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		STUPID_LOG_WARN("%s", message);
		break;

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		STUPID_LOG_INFO("%s", message);
		break;

	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		STUPID_LOG_TRACE("%s", message);
		break;
	default:
		STUPID_LOG_ERROR("unknown message severity");
		STUPID_LOG_ERROR("%s", message);
		break;
	}

	return VK_FALSE;
}

void stRendererVulkanBackendCreateDebugMessenger(StRendererVulkanBackend *pBackend)
{
	STUPID_NC(pBackend);

	VkDebugUtilsMessengerCreateInfoEXT messenger = {0};
	messenger.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messenger.flags = 0;
	messenger.pfnUserCallback = stRendererVulkanDebugCallback;
	messenger.pUserData = NULL;
	messenger.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messenger.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	const PFN_vkCreateDebugUtilsMessengerEXT PFNMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pBackend->instance, "vkCreateDebugUtilsMessengerEXT");
	STUPID_NC(PFNMessenger);
	VK_CHECK(PFNMessenger(pBackend->instance, &messenger, pBackend->pAllocator, &pBackend->debug_messenger));
}
#endif

StRendererVulkanBackend *stRendererVulkanBackendInit(void)
{
	// keeps track of time taken to initialize stRendererVulkan
	StClock c = {0};
	stClockStart(&c);

	StRendererVulkanBackend *pBackend = stMemAlloc(StRendererVulkanBackend, 1);
	pBackend->required_layers = stMemAlloc(const char *, 1);
	pBackend->required_extensions = stWindowGetRequiredExtensions();
	pBackend->required_device_extensions = stMemAlloc(const char *, 3);
	pBackend->pAllocator = NULL;

	// this is basically redundant but the specification requires this to be filled out
	VkApplicationInfo application  = {0};
	application.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application.pApplicationName   = "st_renderer";
	application.pEngineName        = "st_engine";
	application.apiVersion         = VK_API_VERSION_1_3;
	application.engineVersion      = 1;
	application.applicationVersion = 1;

	// enable validation layers if in debug mode
	STUPID_DBG(stMemAppend(pBackend->required_layers, &"VK_LAYER_KHRONOS_validation"));
	STUPID_DBG(stMemAppend(pBackend->required_extensions, &"VK_EXT_debug_utils"));
	stMemAppend(pBackend->required_device_extensions, &VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	stMemAppend(pBackend->required_device_extensions, &VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
	stMemAppend(pBackend->required_device_extensions, &VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

	u32 available_layer_count = 0;
	u32 available_extension_count = 0;

	// check if the required layers are available
	VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, NULL));
	VkLayerProperties *pAvailableLayers = stMemAlloc(VkLayerProperties, available_layer_count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, pAvailableLayers));

	// list the required vulkan layers if they exist
	if (stMemLength(pBackend->required_layers) > 0) {
		STUPID_LOG_DEBUG("required vulkan layers:");
		for (int i = 0; i < stMemLength(pBackend->required_layers); i++)
			STUPID_LOG_DEBUG("        %s", pBackend->required_layers[i]);
	}

	for (int i = 0; i < stMemLength(pBackend->required_layers); i++) {
		bool found = false;
		for (int j = 0; j < available_layer_count; j++) {
			if (stStrneq(pBackend->required_layers[i], pAvailableLayers[j].layerName, 128)) {
				found = true;
				break;
			}
		}
		if (!found) {
			STUPID_LOG_FATAL("vulkan layer '%s' not found", pBackend->required_layers[i]);
			stMemDealloc(pAvailableLayers);
			stMemDealloc(pBackend->required_layers);
			stMemDealloc(pBackend->required_extensions);
			stMemDealloc(pBackend->required_device_extensions);
			stMemDealloc(pBackend->instance);
			stMemDealloc(pBackend);
			return NULL;
		}
	}
	stMemDealloc(pAvailableLayers);

	// check if the required extensions are available
	VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &available_extension_count, NULL));
	VkExtensionProperties *pAvailableExtensions = stMemAlloc(VkExtensionProperties, available_extension_count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(NULL, &available_extension_count, pAvailableExtensions));
	stMemSetLength(pAvailableExtensions, available_extension_count);

	// list the required vulkan extensions if they exist
	if (stMemLength(pBackend->required_extensions) > 0) {
		STUPID_LOG_DEBUG("required vulkan extensions:");
		for (int i = 0; i < stMemLength(pBackend->required_extensions); i++)
			STUPID_LOG_DEBUG("        %s", pBackend->required_extensions[i]);
	}

	for (int i = 0; i < stMemLength(pBackend->required_extensions); i++) {
		bool found = false;
		for (int j = i; j < available_extension_count; j++) {
			if (stStrneq(pBackend->required_extensions[i], pAvailableExtensions[j].extensionName, 128)) {
				found = true;
				break;
			}
		}
		if (!found) {
			STUPID_LOG_FATAL("vulkan extension '%s' not found", pBackend->required_extensions[i]);
			stMemDealloc(pAvailableExtensions);
			stMemDealloc(pBackend->required_layers);
			stMemDealloc(pBackend->required_extensions);
			stMemDealloc(pBackend->required_device_extensions);
			stMemDealloc(pBackend);
			return NULL;
		}
	}
	stMemDealloc(pAvailableExtensions);

	// list the required vulkan device extensions if they exist
	if (stMemLength(pBackend->required_layers) > 0) {
		STUPID_LOG_DEBUG("required vulkan device extensions:");
		for (int i = 0; i < stMemLength(pBackend->required_device_extensions); i++)
			STUPID_LOG_DEBUG("        %s", pBackend->required_device_extensions[i]);
	}

	// instance of the vulkan api
	VkInstanceCreateInfo instance    = {0};
	instance.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance.flags                   = 0;
	instance.pApplicationInfo        = &application;
	instance.enabledLayerCount       = stMemLength(pBackend->required_layers);
	instance.ppEnabledLayerNames     = pBackend->required_layers;
	instance.enabledExtensionCount   = stMemLength(pBackend->required_extensions);
	instance.ppEnabledExtensionNames = pBackend->required_extensions;

	VK_CHECK(vkCreateInstance(&instance, pBackend->pAllocator, &pBackend->instance));

	STUPID_DBG(stRendererVulkanBackendCreateDebugMessenger(pBackend));

	StRendererVulkanDeviceRequirements requirements = {0};
	requirements.queue.graphics  = true;
	requirements.queue.present   = true;
	requirements.queue.transfer  = true;
	requirements.queue.compute   = true;
	requirements.extension_count = stMemLength(pBackend->required_device_extensions);
	requirements.extensions      = pBackend->required_device_extensions;
	requirements.layer_count     = stMemLength(pBackend->required_layers);
	requirements.layers          = pBackend->required_layers;

	if (stRendererVulkanCreateDevice(pBackend->instance, pBackend->pAllocator, &requirements, &pBackend->device) == false) {
		STUPID_LOG_ERROR("unable to find compatible gpu");
		
		const PFN_vkDestroyDebugUtilsMessengerEXT PFNMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pBackend->instance, "vkDestroyDebugUtilsMessengerEXT");
		STUPID_NC(PFNMessenger);
		STUPID_DBG(PFNMessenger(pBackend->instance, pBackend->debug_messenger, pBackend->pAllocator));
		vkDestroyInstance(pBackend->instance, pBackend->pAllocator);
		stMemDealloc(pBackend->required_layers);
		stMemDealloc(pBackend->required_extensions);
		stMemDealloc(pBackend->required_device_extensions);
		stMemDealloc(pBackend);
		return NULL;
	}

	STUPID_LOG_DEBUG("vulkan api version %u.%u.%u",
		VK_API_VERSION_MAJOR(pBackend->device.properties.apiVersion),
		VK_API_VERSION_MINOR(pBackend->device.properties.apiVersion),
		VK_API_VERSION_PATCH(pBackend->device.properties.apiVersion));

	STUPID_LOG_SYSTEM("vulkan backend %p initialized in %f", pBackend, stGetClockElapsed(&c));

	return pBackend;
}

bool stRendererVulkanBackendShutdown(StRendererVulkanBackend *pBackend)
{
	STUPID_NC(pBackend);
	STUPID_NC(pBackend->device.logical_device);

	StClock c = {0};
	stClockStart(&c);

	vkDeviceWaitIdle(pBackend->device.logical_device);

	stRendererVulkanDeviceDestroy(pBackend->pAllocator, &pBackend->device);

#ifdef _DEBUG
	const PFN_vkDestroyDebugUtilsMessengerEXT PFNMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pBackend->instance, "vkDestroyDebugUtilsMessengerEXT");
	STUPID_NC(PFNMessenger);
	PFNMessenger(pBackend->instance, pBackend->debug_messenger, pBackend->pAllocator);
#endif

	vkDestroyInstance(pBackend->instance, pBackend->pAllocator);
	stMemDealloc(pBackend->required_device_extensions);
	stMemDealloc(pBackend->required_extensions);
	stMemDealloc(pBackend->required_layers);

	void *tmp = pBackend;

	stMemDealloc(pBackend);

	STUPID_LOG_SYSTEM("vulkan renderer %p killed in %lf", tmp, stGetClockElapsed(&c));

	return true;
}
