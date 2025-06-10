#include "stupid/render/vulkan/vulkan_frontend.h"
#include "stupid/render/vulkan/vulkan_utils.h"
#include "stupid/render/vulkan/vulkan_command_buffer.h"
#include "stupid/render/vulkan/vulkan_swapchain.h"
#include "stupid/render/vulkan/vulkan_fence.h"
#include "stupid/render/vulkan/vulkan_image.h"
#include "stupid/render/vulkan/vulkan_pipeline.h"

#include "stupid/logger.h"
#include "stupid/window.h"
#include "stupid/memory.h"

#include "stupid/math/linear.h"

StRendererVulkanContext *stRendererVulkanFrontendInit(StRendererVulkanBackend *pBackend, StWindow *pWindow)
{
	STUPID_NC(pBackend);
	STUPID_ASSERT(pWindow != NULL, "headless rendering not yet implemented");

	// allocate all the crap
	StRendererVulkanContext *pContext = stMemAlloc(StRendererVulkanContext, 1);
	pContext->pBackend                = pBackend;
	pContext->pColorAttachments       = stMemAlloc(StRendererVulkanImage, 32);
	pContext->pDepthAttachments       = stMemAlloc(StRendererVulkanImage, 16);
	pContext->pViewports              = stMemAlloc(VkViewport, 8);
	pContext->pScissors               = stMemAlloc(VkRect2D, 8);

	pContext->clear_value.color.float32[0]     = 0.0f;
	pContext->clear_value.color.float32[1]     = 0.0f;
	pContext->clear_value.color.float32[2]     = 0.0f;
	pContext->clear_value.color.float32[3]     = 1.0f;
	pContext->clear_value.depthStencil.depth   = 1.0f;
	pContext->clear_value.depthStencil.stencil = 0;

	pContext->pWindow = pWindow;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	STUPID_ASSERT(stWindowCreateVulkanSurface(pWindow, pBackend->instance, &surface), "failed to create surface");

	i32 width = 0, height = 0;
	stWindowGetSize(pWindow, &width, &height);
	pContext->rvals.width = width;
	pContext->rvals.height = height;

	pWindow->resizing = true;
	STUPID_ASSERT(stRendererVulkanSwapchainCreate(pContext->pBackend, surface, VK_PRESENT_MODE_MAILBOX_KHR, width, height, &pContext->swapchain), "failed to create swapchain");
	pWindow->resizing = false;

	pContext->pRenderingAttachments = stMemAlloc(VkRenderingAttachmentInfo, 8 + pContext->swapchain.image_count);
	pContext->pRenderingAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	pContext->pRenderingAttachments[0].clearValue = pContext->clear_value;
	pContext->pRenderingAttachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	pContext->pRenderingAttachments[0].resolveMode = VK_RESOLVE_MODE_NONE;
	pContext->pRenderingAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	pContext->pRenderingAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	stMemIncrementLength(pContext->pRenderingAttachments);

	if (pContext->swapchain.present_mode == VK_PRESENT_MODE_FIFO_KHR)
		pContext->rvals.vsync = true;

	pContext->pGraphicsCommandBuffers   = stMemAlloc(StRendererVulkanCommandBuffer, pContext->swapchain.image_count);
	pContext->pImageAvailableSemaphores = stMemAlloc(VkSemaphore, pContext->swapchain.image_count + 1);
	pContext->pQueueCompleteSemaphores  = stMemAlloc(VkSemaphore, pContext->swapchain.image_count + 1);
	pContext->pInFlightFences           = stMemAlloc(VkFence, pContext->swapchain.image_count + 1);

	// create semaphores and fences
	for (int i = 0; i < pContext->swapchain.image_count + 1; i++) {
		VkSemaphoreCreateInfo image_available_semaphore_info = {0};
		image_available_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(pContext->pBackend->device.logical_device, &image_available_semaphore_info, pContext->pBackend->pAllocator, &pContext->pImageAvailableSemaphores[i]);

		VkSemaphoreCreateInfo queue_complete_semaphore_info = {0};
		queue_complete_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(pContext->pBackend->device.logical_device, &queue_complete_semaphore_info, pContext->pBackend->pAllocator, &pContext->pQueueCompleteSemaphores[i]);

		stRendererVulkanFenceCreate(pContext->pBackend, true, &pContext->pInFlightFences[i]);
	}

	for (int i = 0; i < pContext->swapchain.image_count; i++) {
		if (pContext->pGraphicsCommandBuffers[i].handle == VK_NULL_HANDLE) {
			stMemset(&pContext->pGraphicsCommandBuffers[i], 0, sizeof(StRendererVulkanCommandBuffer));
			stRendererVulkanCommandBufferAllocate(pContext, pContext->pBackend->device.graphics_command_pool, true, &pContext->pGraphicsCommandBuffers[i]);
	       }
	}

	VkViewport viewport = {0};
	viewport.x          = 0.0f;
	viewport.y          = (f32)height; // opengl shader compatibility (or so ive heard)
	viewport.width      = (f32)width;
	viewport.height     = (f32)height;
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;

	VkRect2D scissor      = {0};
	scissor.extent.width  = width;
	scissor.extent.height = height;
	scissor.offset.x      = 0;
	scissor.offset.y      = 0;

	const char *shader_paths[] = {
		"assets/shaders/shader.vert.spv",
		"assets/shaders/shader.frag.spv"
	};
	VkShaderStageFlagBits stages[] = {
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT
	};
	VkPushConstantRange pRanges[] = {
		{.size = 128, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT},
	};
	stRendererVulkanPipelineCreate(pContext->pBackend, &pContext->graphics_pipeline, pContext->pViewports, 1, pContext->pScissors, 1, &pContext->swapchain.image_format.format, 1, pBackend->device.depth_format, shader_paths, stages, sizeof(stages) / sizeof(VkShaderStageFlagBits), pRanges, sizeof(pRanges) / sizeof(VkPushConstantRange));

	VkPushConstantRange compute_range = {0};
	compute_range.size = 24;
	compute_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	stRendererVulkanPipelineCreateCompute(pContext->pBackend, "assets/shaders/shader.comp.spv", &compute_range, 1, &pContext->compute_pipeline);

	stRendererVulkanImageCreateColor(pBackend,
	                                 true,
	                                 pContext->swapchain.swapchain_width,
	                                 pContext->swapchain.swapchain_height,
	                                 VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	                                 &pContext->pColorAttachments[0]);
	stMemIncrementLength(pContext->pColorAttachments);

	pContext->depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	pContext->depth_attachment.clearValue = pContext->clear_value;
	pContext->depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	pContext->depth_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
	pContext->depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	pContext->depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	pContext->depth_attachment.imageView = pContext->swapchain.depth_attachment.view;

	pContext->rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	pContext->rendering_info.renderArea = (VkRect2D){{0, 0}, {pContext->swapchain.swapchain_width, pContext->swapchain.swapchain_height}};
	pContext->rendering_info.colorAttachmentCount = stMemLength(pContext->pRenderingAttachments);
	pContext->rendering_info.pColorAttachments = pContext->pRenderingAttachments;
	pContext->rendering_info.pDepthAttachment = &pContext->depth_attachment;
	pContext->rendering_info.layerCount = 1;

	stMemAppend(pContext->pViewports, viewport);
	stMemAppend(pContext->pScissors, scissor);

	return pContext;
}

void stRendererVulkanFrontendShutdown(StRendererVulkanContext *pContext)
{
	STUPID_NC(pContext);
	vkDeviceWaitIdle(pContext->pBackend->device.logical_device);
	stRendererVulkanPipelineDestroy(pContext->pBackend, &pContext->graphics_pipeline);
	stRendererVulkanPipelineDestroy(pContext->pBackend, &pContext->compute_pipeline);

	for (int i = 0; i < pContext->swapchain.image_count; i++)
		if (pContext->pGraphicsCommandBuffers[i].handle != VK_NULL_HANDLE)
			stRendererVulkanCommandBufferFree(pContext, pContext->pBackend->device.graphics_command_pool, &pContext->pGraphicsCommandBuffers[i]);

	vkDestroyCommandPool(pContext->pBackend->device.logical_device, pContext->pBackend->device.graphics_command_pool, pContext->pBackend->pAllocator);

	for (int i = 0; i < pContext->swapchain.image_count + 1; i++) {
		if (pContext->pImageAvailableSemaphores[i] != VK_NULL_HANDLE)
			vkDestroySemaphore(pContext->pBackend->device.logical_device, pContext->pImageAvailableSemaphores[i], pContext->pBackend->pAllocator);
		if (pContext->pQueueCompleteSemaphores[i] != VK_NULL_HANDLE)
			vkDestroySemaphore(pContext->pBackend->device.logical_device, pContext->pQueueCompleteSemaphores[i], pContext->pBackend->pAllocator);
		if (pContext->pInFlightFences[i] != VK_NULL_HANDLE)
			stRendererVulkanFenceDestroy(pContext->pBackend, pContext->pInFlightFences[i]);

		pContext->pImageAvailableSemaphores[i] = VK_NULL_HANDLE;
		pContext->pQueueCompleteSemaphores[i] = VK_NULL_HANDLE;
		pContext->pInFlightFences[i] = VK_NULL_HANDLE;

	}

	stRendererVulkanSwapchainDestroy(pContext->pBackend, &pContext->swapchain);

	for (int i = 0; i < stMemLength(pContext->pColorAttachments); i++)
		stRendererVulkanImageDestroy(pContext->pBackend, &pContext->pColorAttachments[i]);

	stMemDealloc(pContext->pInFlightFences);
	stMemDealloc(pContext->pQueueCompleteSemaphores);
	stMemDealloc(pContext->pImageAvailableSemaphores);
	stMemDealloc(pContext->pGraphicsCommandBuffers);
	stMemDealloc(pContext->pScissors);
	stMemDealloc(pContext->pViewports);
	stMemDealloc(pContext->pRenderingAttachments);
	stMemDealloc(pContext->pDepthAttachments);
	stMemDealloc(pContext->pColorAttachments);
	vkDestroySurfaceKHR(pContext->pBackend->instance, pContext->swapchain.surface, pContext->pBackend->pAllocator);
	stMemDealloc(pContext);
}

bool stRendererVulkanFrontendResize(StRendererVulkanContext *pContext, const u32 width, const u32 height)
{
	pContext->rvals.width = width;
	pContext->rvals.height = height;
	if (!stRendererVulkanSwapchainRecreate(pContext->pBackend, width, height, &pContext->swapchain)) return false;
	pContext->pRenderingAttachments[0].imageView = pContext->swapchain.pImages[pContext->image_index].view;
	pContext->depth_attachment.imageView = pContext->swapchain.depth_attachment.view;
	
	// resize the graphics command buffer array if needed
	if (stMemCapacity(pContext->pGraphicsCommandBuffers) > pContext->swapchain.image_count) {
		for (int i = pContext->swapchain.image_count; i < stMemLength(pContext->pGraphicsCommandBuffers); i++)
			stRendererVulkanCommandBufferFree(pContext, pContext->pBackend->device.graphics_command_pool, &pContext->pGraphicsCommandBuffers[i]);
	}
	else {
		for (int i = stMemCapacity(pContext->pGraphicsCommandBuffers); i < pContext->swapchain.image_count; i++) {
			STUPID_LOG_DEBUG("%d %d", i, pContext->swapchain.image_count);
			stMemset(&pContext->pGraphicsCommandBuffers[i], 0, sizeof(StRendererVulkanCommandBuffer));
			stRendererVulkanCommandBufferAllocate(pContext, pContext->pBackend->device.graphics_command_pool, true, &pContext->pGraphicsCommandBuffers[i]);
		}
	}
	return true;
}

bool stRendererVulkanFrontendPrepareFrame(StRendererVulkanContext *pContext, const f32 delta_time)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);
	STUPID_NC(pContext->pBackend->device.logical_device);
	STUPID_NC(pContext->pWindow);

	if (pContext->pWindow->resizing || pContext->swapchain.is_recreating)
		return false;

	// recreate the vulkan swapchain if thats a thing that should happen
	if (pContext->recreating_swapchain) {
		VK_CHECK(vkDeviceWaitIdle(pContext->pBackend->device.logical_device));
		return false;
	}

	if (!stRendererVulkanFenceWait(pContext->pBackend,  &pContext->pInFlightFences[pContext->image_index])) {
		STUPID_LOG_ERROR("stRendererVulkanRendererPrepareFrame(): vulkan fence timed out");
		return false;
	}
	stRendererVulkanFenceReset(pContext->pBackend, &pContext->pInFlightFences[pContext->image_index]);

	if (!stRendererVulkanSwapchainAcquireNextImageIndex(pContext, UINT64_MAX, VK_NULL_HANDLE))
		return false;

	pContext->pCurrentGraphicsCommandBuffer = &pContext->pGraphicsCommandBuffers[pContext->image_index];

	pContext->pViewports[0].width        = (f32)pContext->swapchain.swapchain_width;
	pContext->pViewports[0].height       = (f32)pContext->swapchain.swapchain_height;
	pContext->pViewports[0].x            = 0.0;
	pContext->pViewports[0].y            = (f32)pContext->swapchain.swapchain_height; // TODO: figure out if this is necessary
	pContext->pViewports[0].minDepth     = 0.0;
	pContext->pViewports[0].maxDepth     = 1.0;
	pContext->pScissors[0].extent.width  = pContext->swapchain.swapchain_width;
	pContext->pScissors[0].extent.height = pContext->swapchain.swapchain_height;
	stRendererVulkanCommandBufferReset(pContext->pCurrentGraphicsCommandBuffer);
	stRendererVulkanCommandBufferBegin(false, false, false, pContext->pCurrentGraphicsCommandBuffer);

	vkCmdBindPipeline(pContext->pCurrentGraphicsCommandBuffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pContext->graphics_pipeline.handle);

	vkCmdSetViewport(pContext->pCurrentGraphicsCommandBuffer->handle, 0, 1, &pContext->pViewports[0]);
	vkCmdSetScissor(pContext->pCurrentGraphicsCommandBuffer->handle, 0, 1, &pContext->pScissors[0]);

	stRendererVulkanImageConvert(pContext->pCurrentGraphicsCommandBuffer->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &pContext->swapchain.pImages[pContext->image_index]);

	return true;
}

bool stRendererVulkanFrontendStartFrame(StRendererVulkanContext *pContext, const f32 delta_time)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);
	STUPID_NC(pContext->pBackend->device.logical_device);
	STUPID_NC(pContext->pCurrentGraphicsCommandBuffer);
	STUPID_NC(pContext->pWindow);

	pContext->clear_value.depthStencil.depth = 1.0f;
	pContext->clear_value.depthStencil.stencil = 0;

	pContext->pRenderingAttachments[0].clearValue = pContext->clear_value;
	pContext->depth_attachment.clearValue = pContext->clear_value;
	pContext->rendering_info.renderArea = (VkRect2D){{0, 0}, {pContext->swapchain.swapchain_width, pContext->swapchain.swapchain_height}};
	pContext->rendering_info.colorAttachmentCount = stMemLength(pContext->pRenderingAttachments);

	stRendererVulkanImageConvert(pContext->pCurrentGraphicsCommandBuffer->handle, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &pContext->swapchain.pImages[pContext->image_index]);

	vkCmdBeginRendering(pContext->pCurrentGraphicsCommandBuffer->handle, &pContext->rendering_info);

	pContext->pViewports[0].x = 0;
	pContext->pViewports[0].y = 0;
	pContext->pViewports[0].width = pContext->swapchain.swapchain_width;
	pContext->pViewports[0].height = pContext->swapchain.swapchain_height;
	pContext->pViewports[0].minDepth = 0.0f;
	pContext->pViewports[0].maxDepth = 1.0f;

	pContext->pScissors[0].offset.x = 0;
	pContext->pScissors[0].offset.y = 0;
	pContext->pScissors[0].extent.width = pContext->swapchain.swapchain_width;
	pContext->pScissors[0].extent.height = pContext->swapchain.swapchain_height;

	vkCmdSetViewport(pContext->pCurrentGraphicsCommandBuffer->handle, 0, 1, &pContext->pViewports[0]);
	vkCmdSetScissor(pContext->pCurrentGraphicsCommandBuffer->handle, 0, 1, &pContext->pScissors[0]);

	return true;
}

bool stRendererVulkanFrontendEndFrame(StRendererVulkanContext *pContext, const f32 delta_time)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);
	STUPID_NC(pContext->pBackend->device.logical_device);
	STUPID_NC(pContext->pCurrentGraphicsCommandBuffer);
	STUPID_NC(pContext->pWindow);

	vkCmdEndRendering(pContext->pCurrentGraphicsCommandBuffer->handle);
	stRendererVulkanImageConvert(pContext->pCurrentGraphicsCommandBuffer->handle, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, &pContext->swapchain.pImages[pContext->image_index]);
	stRendererVulkanCommandBufferEnd(pContext->pCurrentGraphicsCommandBuffer);

	VkSubmitInfo submit_info = {0};
	submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pCommandBuffers      = &pContext->pCurrentGraphicsCommandBuffer->handle;
	submit_info.commandBufferCount   = 1;
	submit_info.pSignalSemaphores    = &pContext->pQueueCompleteSemaphores[pContext->current_frame]; // vulkan semaphores that will be signaled when the queues have finished
	submit_info.signalSemaphoreCount = 1;
	submit_info.pWaitSemaphores      = &pContext->pImageAvailableSemaphores[pContext->current_frame]; // vulkan semaphores to wait on until the image is available
	submit_info.waitSemaphoreCount   = 1;

	const VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // VK_PIPELNE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT allows one frame to be presented at a time for some reason
	submit_info.pWaitDstStageMask    = &flags;

	stRendererVulkanFenceReset(pContext->pBackend, &pContext->pInFlightFences[pContext->image_index]);
	const VkResult result = vkQueueSubmit(pContext->pBackend->device.graphics_queue, 1, &submit_info, pContext->pInFlightFences[pContext->image_index]);

	if (!stRendererVulkanResultIsSuccess(result)) {
		STUPID_LOG_ERROR("vulkan queue failed to submit: %s", stRendererVulkanResultStr(result, true));
		return false;
	}

	stRendererVulkanSwapchainPresent(pContext, pContext->pQueueCompleteSemaphores[pContext->current_frame], pContext->image_index);

	pContext->rvals.frames++;

	return true;
}

void stRendererVulkanFrontendDrawRect(StRendererVulkanContext *pContext, const u32 x, const u32 y, const u32 w, const u32 h, const StColor color)
{
	STUPID_NC(pContext);

	if (pContext->pCurrentGraphicsCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING)
		return;

	VkClearAttachment clear_attachment = {0};
	clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	clear_attachment.colorAttachment = 0;
	clear_attachment.clearValue.color.float32[0] = color.r;
	clear_attachment.clearValue.color.float32[1] = color.g;
	clear_attachment.clearValue.color.float32[2] = color.b;
	clear_attachment.clearValue.color.float32[3] = color.a;

	VkClearRect clear_rect = {0};
	clear_rect.rect.extent.width  = w;
	clear_rect.rect.extent.height = h;
	clear_rect.rect.offset.x      = x;
	clear_rect.rect.offset.y      = y;
	clear_rect.baseArrayLayer     = 0;
	clear_rect.layerCount         = 1;

	vkCmdClearAttachments(pContext->pCurrentGraphicsCommandBuffer->handle, 1, &clear_attachment, 1, &clear_rect);
}

void stRendererVulkanFrontendClear(StRendererVulkanContext *pContext)
{
	STUPID_NC(pContext);

	if (pContext->pCurrentGraphicsCommandBuffer->state != ST_RENDERER_VULKAN_COMMAND_BUFFER_STATE_RECORDING)
		return;

	const StColor color = {
		.r = pContext->clear_value.color.float32[0],
		.g = pContext->clear_value.color.float32[1],
		.b = pContext->clear_value.color.float32[2],
		.a = pContext->clear_value.color.float32[3]
	};

	stRendererVulkanImageClear(pContext->pCurrentGraphicsCommandBuffer->handle, color, &pContext->swapchain.pImages[pContext->image_index]);
}

void stRendererVulkanFrontendSetVsync(StRendererVulkanContext *pContext, const bool state)
{
	STUPID_NC(pContext);
	STUPID_NC(pContext->pBackend);

	if (state) {
		if (pContext->swapchain.present_mode == VK_PRESENT_MODE_FIFO_KHR)
			return;

		pContext->swapchain.previous_present_mode = pContext->swapchain.present_mode;
		pContext->swapchain.present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}
	else {
		if (pContext->swapchain.present_mode != VK_PRESENT_MODE_FIFO_KHR)
			return;

		pContext->swapchain.present_mode = pContext->swapchain.previous_present_mode;
		pContext->swapchain.previous_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	stRendererVulkanSwapchainRecreate(pContext->pBackend, pContext->swapchain.swapchain_width, pContext->swapchain.swapchain_height, &pContext->swapchain);
}

void stRendererVulkanFrontendPrepareModelMatrices(StRendererVulkanContext *pContext, usize model_count, StRendererBuffer *pModelBuffer, StRendererBuffer *pTransformationBuffer)
{
	STUPID_NC(pContext);
	STUPID_NC(pModelBuffer);
	STUPID_NC(pTransformationBuffer);

	struct pc {
		VkDeviceAddress models;
		VkDeviceAddress transformations;
		u32 model_count;
	} pc;

	StRendererVulkanBuffer *models = pModelBuffer->internal;
	StRendererVulkanBuffer *transformations = pTransformationBuffer->internal;

	pc.models = models->address;
	pc.transformations = transformations->address;
	pc.model_count = model_count;

	vkCmdBindPipeline(pContext->pCurrentGraphicsCommandBuffer->handle, VK_PIPELINE_BIND_POINT_COMPUTE, pContext->compute_pipeline.handle);
	vkCmdPushConstants(pContext->pCurrentGraphicsCommandBuffer->handle, pContext->compute_pipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pc), &pc);
	u32 workgroup_size = 64;
	u32 workgroup_count = (model_count + workgroup_size - 1) / workgroup_size;
	vkCmdDispatch(pContext->pCurrentGraphicsCommandBuffer->handle, workgroup_count, 1, 1);
}

void stRendererVulkanFrontendDrawObjects(StRendererVulkanContext *pContext, const StMat4 view_projection, const usize object_count, StRendererBuffer *pPositionBuffer, StRendererBuffer *pIndexBuffer, StRendererBuffer *pModelBuffer, StObject *pObjects)
{
	STUPID_NC(pContext);
	STUPID_NC(pPositionBuffer);
	STUPID_NC(pIndexBuffer);
	STUPID_NC(pModelBuffer);
	STUPID_NC(pObjects);
	
	struct pc {
		StMat4 view_projection;
		StVec4 pos;
		StVec4 target;
		VkDeviceAddress positions;
		VkDeviceAddress indices;
		VkDeviceAddress models;
		u32 id;
	} pc = {0};

	StRendererVulkanBuffer *positions = pPositionBuffer->internal;
	StRendererVulkanBuffer *indices = pIndexBuffer->internal;
	StRendererVulkanBuffer *models = pModelBuffer->internal;

	pc.view_projection = view_projection;
	pc.pos = STVEC4(pContext->rvals.camera.pos.x, pContext->rvals.camera.pos.y, pContext->rvals.camera.pos.z, 1.0);
	pc.target = STVEC4(pContext->rvals.camera.target.x, pContext->rvals.camera.target.y, pContext->rvals.camera.target.z, 1.0);
	pc.models = models->address;

	vkCmdBindPipeline(pContext->pCurrentGraphicsCommandBuffer->handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pContext->graphics_pipeline.handle);

	for (usize i = 0; i < object_count; i++) {
		pc.positions = positions->address + pObjects[i].position_offset;
		pc.indices = indices->address + pObjects[i].index_offset;
		pc.id = pObjects[i].transformation_index;

		vkCmdPushConstants(pContext->pCurrentGraphicsCommandBuffer->handle, pContext->graphics_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
		vkCmdDraw(pContext->pCurrentGraphicsCommandBuffer->handle, pObjects[i].index_count, 1, 0, 0);
	}
}

