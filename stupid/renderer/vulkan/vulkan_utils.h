#pragma once

#include <vulkan/vulkan.h>
#include "common.h"

#include "core/asserts.h"
#include "core/event.h"

/**
 * @brief Gets a string explaining a Vulkan error code.
 * @param result VkResult value.
 * @param verbose Whether extra information should be included explaining the result.
 * @return String corresponding to the error code.
 * @note From https://github.com/travisvroman/kohi/blob/8be74ae43f8a2b0a3146040a7aea1eeb842b3f10/kohi.plugin.renderer.stRendererVulkan/src/vulkan_utils.c
 */
const char *stRendererVulkanResultStr(const VkResult result, const bool verbose);

/**
 * Checks if a Vulkan result is a successful result.
 * @param result StRendererVulkan result.
 * @return True if the result does not indicate failure.
 */
static inline bool stRendererVulkanResultIsSuccess(const VkResult result)
{
        // from https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkResult.html
        switch (result) {
                case VK_SUCCESS:
                case VK_NOT_READY:
                case VK_TIMEOUT:
                case VK_EVENT_SET:
                case VK_EVENT_RESET:
                case VK_SUBOPTIMAL_KHR:
                case VK_THREAD_IDLE_KHR:
                case VK_THREAD_DONE_KHR:
                case VK_OPERATION_DEFERRED_KHR:
                case VK_OPERATION_NOT_DEFERRED_KHR:
                case VK_PIPELINE_COMPILE_REQUIRED_EXT:
                        return true;

                case VK_ERROR_OUT_OF_HOST_MEMORY:
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                case VK_ERROR_INITIALIZATION_FAILED:
                case VK_ERROR_DEVICE_LOST:
                case VK_ERROR_MEMORY_MAP_FAILED:
                case VK_ERROR_LAYER_NOT_PRESENT:
                case VK_ERROR_EXTENSION_NOT_PRESENT:
                case VK_ERROR_FEATURE_NOT_PRESENT:
                case VK_ERROR_INCOMPATIBLE_DRIVER:
                case VK_ERROR_TOO_MANY_OBJECTS:
                case VK_ERROR_FORMAT_NOT_SUPPORTED:
                case VK_ERROR_FRAGMENTED_POOL:
                case VK_ERROR_SURFACE_LOST_KHR:
                case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                case VK_ERROR_OUT_OF_DATE_KHR:
                case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                case VK_ERROR_INVALID_SHADER_NV:
                case VK_ERROR_OUT_OF_POOL_MEMORY:
                case VK_ERROR_INVALID_EXTERNAL_HANDLE:
                case VK_ERROR_FRAGMENTATION:
                case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
                case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
                case VK_INCOMPLETE:
                case VK_ERROR_UNKNOWN:
                default:
                        return false;
        }
}

/**
 * Sends STUPID_EVENT_CODE_FATAL_ERROR if the given function did not finish successfully.
 * @param expr A function that returns VkResult.
 */
#define VK_CHECK(expr)\
        do {\
                VkResult result = (expr);\
                if (!stRendererVulkanResultIsSuccess(result)) {\
                        STUPID_LOG_FATAL("%s: '%s' %s:%d", #expr, stRendererVulkanResultStr(result, STUPID_DBG_IF_ELSE(true, false)), __FILE__, __LINE__);\
                        stEventFire(STUPID_EVENT_CODE_FATAL_ERROR, (StEventData){0});\
                        STUPID_STOP();\
                }\
        } while (0)
