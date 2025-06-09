#include "stupid/renderer/vulkan/vulkan_utils.h"

const char *stRendererVulkanResultStr(const VkResult result, const bool verbose)
{
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkResult.html
	switch (result) {
	default:
	case VK_SUCCESS:
		return !verbose ? "VK_SUCCESS" : "VK_SUCCESS command successfully completed";
	case VK_NOT_READY:
		return !verbose ? "VK_NOT_READY" : "VK_NOT_READY a fence or query has not yet completed";
	case VK_TIMEOUT:
		return !verbose ? "VK_TIMEOUT" : "VK_TIMEOUT a wait operation has not completed in the specified time";
	case VK_EVENT_SET:
		return !verbose ? "VK_EVENT_SET" : "VK_EVENT_SET an event is signaled";
	case VK_EVENT_RESET:
		return !verbose ? "VK_EVENT_RESET" : "VK_EVENT_RESET an event is unsignaled";
	case VK_SUBOPTIMAL_KHR:
		return !verbose ? "VK_SUBOPTIMAL_KHR"
		                  : "VK_SUBOPTIMAL_KHR a swapchain no longer matches the surface properties exactly but can still be used "
		                    "to present to the surface successfully";
	case VK_THREAD_IDLE_KHR:
		return !verbose ? "VK_THREAD_IDLE_KHR"
		                  : "VK_THREAD_IDLE_KHR a deferred operation is not complete but there is currently no work for this "
		                    "thread to do at the time of this call";
	case VK_THREAD_DONE_KHR:
		return !verbose ? "VK_THREAD_DONE_KHR"
		                  : "VK_THREAD_DONE_KHR a deferred operation is not complete but there is no work remaining to assign to "
		                    "additional threads";
	case VK_OPERATION_DEFERRED_KHR:
		return !verbose
		               ? "VK_OPERATION_DEFERRED_KHR"
		               : "VK_OPERATION_DEFERRED_KHR a deferred operation was requested and at least some of the work was deferred";
	case VK_OPERATION_NOT_DEFERRED_KHR:
		return !verbose ? "VK_OPERATION_NOT_DEFERRED_KHR"
		                  : "VK_OPERATION_NOT_DEFERRED_KHR a deferred operation was requested and no operations were deferred";
	case VK_PIPELINE_COMPILE_REQUIRED_EXT:
		return !verbose ? "VK_PIPELINE_COMPILE_REQUIRED_EXT"
		                  : "VK_PIPELINE_COMPILE_REQUIRED_EXT a requested pipeline creation would have required compilation but "
		                    "the application requested compilation to not be performed";

	// error codes
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return !verbose ? "VK_ERROR_OUT_OF_HOST_MEMORY" : "VK_ERROR_OUT_OF_HOST_MEMORY a host memory allocation has failed";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return !verbose ? "VK_ERROR_OUT_OF_DEVICE_MEMORY"
		                  : "VK_ERROR_OUT_OF_DEVICE_MEMORY a device memory allocation has failed";
	case VK_ERROR_INITIALIZATION_FAILED:
		return !verbose ? "VK_ERROR_INITIALIZATION_FAILED"
		                  : "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could not be completed for "
		                    "implementation-specific reasons";
	case VK_ERROR_DEVICE_LOST:
		return !verbose ? "VK_ERROR_DEVICE_LOST"
		                  : "VK_ERROR_DEVICE_LOST The logical or physical device has been lost. See Lost Device";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return !verbose ? "VK_ERROR_MEMORY_MAP_FAILED" : "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return !verbose ? "VK_ERROR_LAYER_NOT_PRESENT"
		                  : "VK_ERROR_LAYER_NOT_PRESENT a requested layer is not present or could not be loaded";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return !verbose ? "VK_ERROR_EXTENSION_NOT_PRESENT"
		                  : "VK_ERROR_EXTENSION_NOT_PRESENT a requested extension is not supported";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return !verbose ? "VK_ERROR_FEATURE_NOT_PRESENT" : "VK_ERROR_FEATURE_NOT_PRESENT a requested feature is not supported";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return !verbose ? "VK_ERROR_INCOMPATIBLE_DRIVER"
		                  : "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of vulkan is not supported by the driver or is "
		                    "otherwise incompatible for implementation-specific reasons";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return !verbose ? "VK_ERROR_TOO_MANY_OBJECTS"
		                  : "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have already been created";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return !verbose ? "VK_ERROR_FORMAT_NOT_SUPPORTED"
		                  : "VK_ERROR_FORMAT_NOT_SUPPORTED a requested format is not supported on this device";
	case VK_ERROR_FRAGMENTED_POOL:
		return !verbose ? "VK_ERROR_FRAGMENTED_POOL"
		                  : "VK_ERROR_FRAGMENTED_POOL a pool allocation has failed due to fragmentation of the pool’s memory. This "
		                    "must only be returned if no attempt to allocate host or device memory was made to accommodate the new "
		                    "allocation. This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY but only if the "
		                    "implementation is certain that the pool allocation failure was due to fragmentation";
	case VK_ERROR_SURFACE_LOST_KHR:
		return !verbose ? "VK_ERROR_SURFACE_LOST_KHR" : "VK_ERROR_SURFACE_LOST_KHR a surface is no longer available";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return !verbose ? "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"
		                  : "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already in use by vulkan or another API in "
		                    "a manner which prevents it from being used again";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return !verbose
		               ? "VK_ERROR_OUT_OF_DATE_KHR"
		               : "VK_ERROR_OUT_OF_DATE_KHR a surface has changed in such a way that it is no longer compatible with the "
		                 "swapchain and further presentation requests using the swapchain will fail. Applications must query the "
		                 "new surface properties and recreate their swapchain if they wish to continue presenting to the surface";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return !verbose ? "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"
		                  : "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain does not use the same presentable "
		                    "image layout or is incompatible in a way that prevents sharing an image";
	case VK_ERROR_INVALID_SHADER_NV:
		return !verbose ? "VK_ERROR_INVALID_SHADER_NV"
		                  : "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile or link. More details are reported "
		                    "back to the application via VK_EXT_debug_report if enabled";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return !verbose ? "VK_ERROR_OUT_OF_POOL_MEMORY"
		                  : "VK_ERROR_OUT_OF_POOL_MEMORY a pool memory allocation has failed. This must only be returned if no "
		                    "attempt to allocate host or device memory was made to accommodate the new allocation. If the failure "
		                    "was definitely due to fragmentation of the pool VK_ERROR_FRAGMENTED_POOL should be returned instead";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return !verbose ? "VK_ERROR_INVALID_EXTERNAL_HANDLE"
		                  : "VK_ERROR_INVALID_EXTERNAL_HANDLE an external handle is not a valid handle of the specified type";
	case VK_ERROR_FRAGMENTATION:
		return !verbose ? "VK_ERROR_FRAGMENTATION"
		                  : "VK_ERROR_FRAGMENTATION a descriptor pool creation has failed due to fragmentation";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
		return !verbose ? "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT"
		                  : "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT a buffer creation failed because the requested address is not available";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		return !verbose ? "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"
		                  : "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT an operation on a swapchain created with "
		                    "VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT failed as it did not have exclusive full-screen "
		                    "access. this may occur due to implementation-dependent reasons outside of the application’s control";
	case VK_INCOMPLETE:
		return !verbose ? "VK_INCOMPLETE" : "VK_INCOMPLETE a return array was too small for the result";
	case VK_ERROR_UNKNOWN:
		return !verbose ? "VK_ERROR_UNKNOWN"
		                  : "VK_ERROR_UNKNOWN an unknown error has occurred; either the application has provided invalid input or "
		                    "an implementation failure has occurred";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return !verbose ? "VK_ERROR_VALIDATION_FAILED_EXT"
		                  : "A command failed because invalid usage was detected by the implementation or a validation-layer";

		break;
	}
}
