#pragma once

#include "stupid/common.h"
#include "stupid/window.h"
#include "stupid/render/vulkan/vulkan_types.h"

/**
 * Shuts down a vulkan frontend instnace.
 * @param pBackend Pointer to a vulkan backend.
 * @param pWindow Pointer to a window this vulkan frontend instance will be bound to.
 * @return A new vulkan frontend instance.
 */
StRendererVulkanContext *stRendererVulkanFrontendInit(StRendererVulkanBackend *pBackend, StWindow *pWindow);

/**
 * Shuts down a vulkan frontend instnace.
 * @param pContext Pointer to a vulkan renderer instance.
 */
void stRendererVulkanFrontendShutdown(StRendererVulkanContext *pContext);

/**
 * Resizes the vulkan renderer.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param width New swapchain width.
 * @param height New swapchain height.
 * @return True if successful.
 * @see stRendererResize
 */
bool stRendererVulkanFrontendResize(StRendererVulkanContext *pContext, const u32 width, const u32 height);

/**
 * Enters the preparation phase of the next frame and waits for the last one to finish.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param delta_time Time since last frame.
 * @return True if successful.
 * @see stRendererPrepareFrame, stRendererVulkanStartFrame
 */
bool stRendererVulkanFrontendPrepareFrame(StRendererVulkanContext *pContext, const f32 delta_time);

/**
 * Starts rendering a frame.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param delta_time Time since last frame.
 * @return True if successful.
 * @see stRendererStartFrame, stRendererVulkanEndFrame
 */
bool stRendererVulkanFrontendStartFrame(StRendererVulkanContext *pContext, const f32 delta_time);

/**
 * Finishes rendering a frame.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param delta_time Time since last frame.
 * @return True if successful.
 * @see stRendererEndFrame
 */
bool stRendererVulkanFrontendEndFrame(StRendererVulkanContext *pContext, const f32 delta_time);

/**
 * Sets the default clear color of a vulkan renderer instance.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param color New clear color.
 */
static STUPID_INLINE void stRendererVulkanFrontendSetClearColor(StRendererVulkanContext *pContext, const StColor color)
{
        pContext->clear_value.color.float32[0] = color.r;
        pContext->clear_value.color.float32[1] = color.g;
        pContext->clear_value.color.float32[2] = color.b;
        pContext->clear_value.color.float32[3] = color.a;
}

/**
 * Renders a rect of the specified color.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param x Horizontal position.
 * @param y Vertical position.
 * @param w Horizontal size.
 * @param h Vertical size.
 * @param color Specified color.
 */
void stRendererVulkanFrontendDrawRect(StRendererVulkanContext *pContext, const u32 x, const u32 y, const u32 w, const u32 h, const StColor color);

/**
 * Clears the current frame.
 * @param pContext Pointer to a vulkan renderer instance.
 * @see stRendererClear
 */
void stRendererVulkanFrontendClear(StRendererVulkanContext *pContext);

/**
 * Toggles VSync.
 * @param pContext Pointer to a vulkan renderer instance.
 * @pointer state True to enable VSync, false to disable it.
 */
void stRendererVulkanFrontendSetVsync(StRendererVulkanContext *pContext, const bool state);

void stRendererVulkanFrontendPrepareModelMatrices(StRendererVulkanContext *pContext, usize model_count, StRendererBuffer *pModelBuffer, StRendererBuffer *pTransformationBuffer);

/**
 * Draws an object.
 * @param pContext Pointer to a vulkan renderer instance.
 * @param pObject Object to draw.
 * @param mvp Model-view-projection matrix.
 * @param rotation Rotation and translation to apply to the object.
 */
void stRendererVulkanFrontendDrawObjects(StRendererVulkanContext *pContext, const StMat4 view_projection, const usize object_count, StRendererBuffer *pPositionBuffer, StRendererBuffer *pIndexBuffer, StRendererBuffer *pModelBuffer, StObject *pObjects);

