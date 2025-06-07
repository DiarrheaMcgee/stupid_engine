#include "common.h"

#include "render.h"
#include "core/clock.h"
#include "core/thread.h"
#include "math/linear.h"
#include "render_types.h"

#include "renderer/vulkan/vulkan_backend.h"
#include "renderer/vulkan/vulkan_memory.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_frontend.h"

#include "core/asserts.h"
#include "core/event.h"

#include "memory/memory.h"
#include <vulkan/vulkan_core.h>

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

static STUPID_INLINE StRendererValues *getRvals(const StRenderer *pRenderer)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->pRendererInstance);
	return &((StRendererVulkanContext *)pRenderer->pRendererInstance)->rvals;
}

static bool handleResize(const st_event_code code, void *sender, void *listener, const StEventData data)
{
	STUPID_NC(listener);
	StRenderer *pRenderer = listener;

	switch (pRenderer->type) {
		default: {
			StRendererVulkanContext *pContext = pRenderer->pRendererInstance;
			if (pContext->pWindow != sender) return false;
			break;
		}
	}

	u32 timeout = 0;

	// this waits until the renderer isnt in the middle of rendering a frame before it resizes (max wait time 200ms)
	while (pRenderer->state != ST_RENDERER_STATE_IDLE) {
		stSleep(1);
		if (timeout++ >= 200) break;
	}
	return stRendererResize(listener, data.window.w, data.window.h);
}

/// Container for a renderer backend (just so the type can be stored).
typedef struct RendererBackend {
	void *pBackend;
	st_renderer_backend type;
} RendererBackend;

static bool vulkan_initialized = false;

void *stRendererBackendInitialize(const st_renderer_backend backend)
{
	STUPID_ASSERT(backend >= ST_RENDERER_BACKEND_UNDEFINED && backend < ST_RENDERER_BACKEND_MAX,
	              "invalid stStRenderer backend");

	switch (backend) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN:
			STUPID_ASSERT(!vulkan_initialized, "vulkan backend already initialized");
			RendererBackend *pContainer = stMemAllocNL(RendererBackend, 1);
			pContainer->pBackend = stRendererVulkanBackendInit();
			pContainer->type = ST_RENDERER_BACKEND_VULKAN;
			vulkan_initialized = true;
			return pContainer;
			break;
		default:
			// unreachable
			break;
	}
	return NULL;
}

void stRendererBackendShutdown(void *pBackend)
{
	STUPID_NC(pBackend);

	RendererBackend *pContainer = pBackend;
	const st_renderer_backend backend = pContainer->type;
	STUPID_ASSERT(backend >= ST_RENDERER_BACKEND_UNDEFINED && backend < ST_RENDERER_BACKEND_MAX,
	              "invalid stStRenderer backend");

	switch (backend) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN:
			STUPID_ASSERT(pContainer->pBackend != NULL && vulkan_initialized,
			              "vulkan backend not initialized");
			stRendererVulkanBackendShutdown(pContainer->pBackend);
			stMemDeallocNL(pBackend);
			vulkan_initialized = false;
			break;
		default:
			// unreachable
			break;
	}
}

StRenderer *stRendererCreate(void *pBackend, StWindow *pWindow)
{
	STUPID_NC(pBackend);

	RendererBackend *pContainer = pBackend;
	const st_renderer_backend backend = pContainer->type;
	STUPID_ASSERT(backend >= ST_RENDERER_BACKEND_UNDEFINED && backend < ST_RENDERER_BACKEND_MAX,
	              "invalid stStRenderer backend");

	StRenderer *pRenderer = stMemAlloc(StRenderer, 1);

	switch (backend) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN:
			pRenderer->PFNInit = (StPFN_renderer_init)stRendererVulkanFrontendInit;
			pRenderer->PFNShutdown = (StPFN_renderer_shutdown)stRendererVulkanFrontendShutdown;
			pRenderer->PFNResize = (StPFN_renderer_resize)stRendererVulkanFrontendResize;
			pRenderer->PFNPrepareFrame = (StPFN_renderer_prepare_frame)stRendererVulkanFrontendPrepareFrame;
			pRenderer->PFNStartFrame = (StPFN_renderer_start_frame)stRendererVulkanFrontendStartFrame;
			pRenderer->PFNEndFrame = (StPFN_renderer_end_frame)stRendererVulkanFrontendEndFrame;
			pRenderer->PFNSetClearColor = (StPFN_renderer_set_clear_color)stRendererVulkanFrontendSetClearColor;
			pRenderer->PFNClear = (StPFN_renderer_clear)stRendererVulkanFrontendClear;
			pRenderer->PFNDrawRect = (StPFN_renderer_draw_rect)stRendererVulkanFrontendDrawRect;
			pRenderer->PFNSetVsync = (StPFN_renderer_set_vsync)stRendererVulkanFrontendSetVsync;
			pRenderer->PFNDrawObjects = (StPFN_renderer_draw_objects)stRendererVulkanFrontendDrawObjects;
			pRenderer->PFNPrepareModelMatrices = (StPFN_renderer_prepare_model_matrices)stRendererVulkanFrontendPrepareModelMatrices;
			break;
		default:
			break;
	}

	pRenderer->pRendererInstance = pRenderer->PFNInit(pContainer->pBackend, pWindow);
	pRenderer->PFNSetClearColor(pRenderer->pRendererInstance, (StColor){0.0f, 0.0f, 0.0f, 1.0f});
	pRenderer->rvals = getRvals(pRenderer);
	stEventRegister(STUPID_EVENT_CODE_WINDOW_RESIZED, pRenderer, handleResize);

	stRendererAllocate(pRenderer, STUPID_RENDERER_OBJECT_POSITION_BUFFER_SIZE, ST_RENDERER_BUFFER_USAGE_GENERIC, &pRenderer->positions);
	stRendererAllocate(pRenderer, STUPID_RENDERER_OBJECT_INDEX_BUFFER_SIZE, ST_RENDERER_BUFFER_USAGE_GENERIC, &pRenderer->indices);
	stRendererAllocate(pRenderer, STUPID_RENDERER_MAX_OBJECTS * sizeof(StMat4) * 2, ST_RENDERER_BUFFER_USAGE_GENERIC, &pRenderer->models);
	stRendererAllocate(pRenderer, STUPID_RENDERER_MAX_OBJECTS * sizeof(StVec3) * 3, ST_RENDERER_BUFFER_USAGE_GENERIC | ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_FAST, &pRenderer->transformations);

	StVec3 *map = stRendererMap(pRenderer, &pRenderer->transformations);
	for (int i = 0; i < STUPID_RENDERER_MAX_OBJECTS; i++) {
		map[i * 3] = STVEC3(0.0, 0.0, 0.0);
		map[i * 3 + 1] = STVEC3(0.0, 0.0, 0.0);
		map[i * 3 + 2] = STVEC3(1.0, 1.0, 1.0);
	}

	pRenderer->rvals->camera = stRendererCameraCreate(STVEC3(0.0, 0.0, 0.0), STVEC3(0.0, 0.0, -1.0), 1.0, 0.01, 100.0);

	return pRenderer;
}

void stRendererDestroy(StRenderer *pRenderer)
{
	STUPID_NC(pRenderer);
	stRendererUnmap(pRenderer, &pRenderer->transformations);
	stRendererDeallocate(pRenderer, &pRenderer->transformations);
	stRendererDeallocate(pRenderer, &pRenderer->models);
	stRendererDeallocate(pRenderer, &pRenderer->indices);
	stRendererDeallocate(pRenderer, &pRenderer->positions);
	stEventUnregister(STUPID_EVENT_CODE_WINDOW_RESIZED, pRenderer, handleResize);
	pRenderer->PFNShutdown(pRenderer->pRendererInstance);
	stMemDealloc(pRenderer);
}

void (stRendererAllocate)(StRenderer *pRenderer, const usize size, const st_renderer_buffer_flags flags, StRendererBuffer *pBuffer STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pBuffer);

	stMemset(pBuffer, 0, sizeof(StRendererBuffer));

	STUPID_ASSERT(size >= 8, "creating a vram allocation that small is just a waste of everybodys time");
	//STUPID_ASSERT(type >= 0 && type < ST_RENDERER_BUFFER_TYPE_MAX, "invalid buffer type");
	STUPID_ASSERT(!(flags & ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_SLOW && flags & ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_FAST), "cant enable slow cpu access and fast cpu access at the same time");

	void *address = NULL;

	switch (pRenderer->type) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN: {
			StRendererVulkanContext *pContext = pRenderer->pRendererInstance;
			pBuffer->internal = stMemAllocNL(StRendererVulkanBuffer, 1);

			u32 usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (flags & ST_RENDERER_BUFFER_USAGE_GENERIC)
				usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

			if (flags & ST_RENDERER_BUFFER_USAGE_VERTEX)
				usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

			if (flags & ST_RENDERER_BUFFER_USAGE_INDEX)
				usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

			if (flags & ST_RENDERER_BUFFER_USAGE_TEXTURE)
				usage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

			if (flags & ST_RENDERER_BUFFER_USAGE_TRANSFER_SOURCE)
				usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			u32 location = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			if (flags & ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_SLOW)
				location = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			else if (flags & ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_FAST)
				location |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			stRendererVulkanMemoryAllocate(pContext->pBackend, size, usage, location, pBuffer->internal);

			const StRendererVulkanBuffer *internal = pBuffer->internal;
			address = (void *)internal->address;

			break;
		}
		case ST_RENDERER_BACKEND_MAX:
			// unreachable
			break;
	}

	if (size >= sizeof(StGb))
		STUPID_LOG_TRACEFN("allocated buffer %p size %zu (%.2fG)", address, size, (f64)size / (f64)sizeof(StGb));
	else if (size >= sizeof(StMb))
		STUPID_LOG_TRACEFN("allocated buffer %p size %zu (%.2fM)", address, size, (f64)size / (f64)sizeof(StMb));
	else if (size >= sizeof(StKb))
		STUPID_LOG_TRACEFN("allocated buffer %p size %zu (%.2fK)", address, size, (f64)size / (f64)sizeof(StKb));
	else
		STUPID_LOG_TRACEFN("allocated buffer %p size %zu", address, size);
}

void (stRendererDeallocate)(StRenderer *pRenderer, StRendererBuffer *pBuffer STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pBuffer);
	STUPID_NC(pBuffer->internal);

	usize size = 0;
	void *address = NULL;

	switch (pRenderer->type) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN: {
			StRendererVulkanContext *pContext = pRenderer->pRendererInstance;
			StRendererVulkanBuffer *internal = pBuffer->internal;
			pBuffer->map = NULL;
			size = internal->size;
			address = (void *)internal->address;
			stRendererVulkanMemoryDeallocate(pContext->pBackend, pBuffer->internal);
			stMemDeallocNL(pBuffer->internal);
			break;
		}
		case ST_RENDERER_BACKEND_MAX:
			// unreachable
			break;
	}

        if (size >= sizeof(StGb))
                STUPID_LOG_TRACEFN("deallocated buffer %p size %zu (%.2fG)", address, size, (f64)size / (f64)sizeof(StGb));
        else if (size >= sizeof(StMb))
                STUPID_LOG_TRACEFN("deallocated buffer %p size %zu (%.2fM)", address, size, (f64)size / (f64)sizeof(StMb));
        else if (size >= sizeof(StKb))
                STUPID_LOG_TRACEFN("deallocated buffer %p size %zu (%.2fK)", address, size, (f64)size / (f64)sizeof(StKb));
        else
                STUPID_LOG_TRACEFN("deallocated buffer %p size %zu", address, size);
}

void *stRendererMap(StRenderer *pRenderer, StRendererBuffer *pBuffer)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pBuffer);
	STUPID_NC(pBuffer->internal);

	switch (pRenderer->type) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN: {
			StRendererVulkanContext *pContext = pRenderer->pRendererInstance;
			stRendererVulkanMemoryMap(pContext->pBackend, &pBuffer->map, pBuffer->internal);
			break;
		}
		case ST_RENDERER_BACKEND_MAX:
			// unreachable
			break;
	}

	return pBuffer->map;
}

void stRendererUnmap(StRenderer *pRenderer, StRendererBuffer *pBuffer)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pBuffer);
	STUPID_NC(pBuffer->internal);

	switch (pRenderer->type) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN: {
			StRendererVulkanContext *pContext = pRenderer->pRendererInstance;
			stRendererVulkanMemoryUnmap(pContext->pBackend, pBuffer->internal);
			break;
		}
		case ST_RENDERER_BACKEND_MAX:
			// unreachable
			break;
	}

	pBuffer->map = NULL;
}

void stRendererFlushMap(StRenderer *pRenderer, StRendererBuffer *pBuffer)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pBuffer);
	STUPID_NC(pBuffer->internal);

	switch (pRenderer->type) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN: {
			StRendererVulkanContext *pContext = pRenderer->pRendererInstance;
			StRendererVulkanBuffer *internal = pBuffer->internal;
			VkMappedMemoryRange range = {0};
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.memory = internal->memory;
			range.size = internal->size;
			vkFlushMappedMemoryRanges(pContext->pBackend->device.logical_device, 1, &range);
			break;
		}

		case ST_RENDERER_BACKEND_MAX:
			break;
	}
}

bool stRendererResize(StRenderer *pRenderer, const i32 width, const i32 height)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNResize);

	stMutexLock(&pRenderer->lock);

	if (pRenderer->state == ST_RENDERER_STATE_FRAME_PREPARE || pRenderer->state == ST_RENDERER_STATE_FRAME_START ||
	    pRenderer->state == ST_RENDERER_STATE_FRAME_END) {
		STUPID_LOG_ERROR("stRendererResize() called mid-frame");
		stMutexUnlock(&pRenderer->lock);
		return false;
	}

	const bool res = pRenderer->PFNResize(pRenderer->pRendererInstance, width, height);
	stMutexUnlock(&pRenderer->lock);
	return res;
}

bool stRendererPrepareFrame(StRenderer *pRenderer, f32 delta_time)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNPrepareFrame);

	//stMutexLock(&pRenderer->lock);

	if (pRenderer->state == ST_RENDERER_STATE_FRAME_PREPARE || pRenderer->state == ST_RENDERER_STATE_FRAME_START ||
	    pRenderer->state == ST_RENDERER_STATE_FRAME_END) {
		STUPID_LOG_ERROR("stRendererPrepareFrame() called twice");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}
	else if (STUPID_IS_NAN(delta_time)) {
		STUPID_LOG_ERROR("stRendererPrepareFrame() called with NAN delta_time");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}

	pRenderer->state = ST_RENDERER_STATE_FRAME_PREPARE;

	const bool res = pRenderer->PFNPrepareFrame(pRenderer->pRendererInstance, delta_time);
	pRenderer->PFNPrepareModelMatrices(pRenderer->pRendererInstance, STUPID_RENDERER_MAX_OBJECTS, &pRenderer->models, &pRenderer->transformations);

	//stMutexUnlock(&pRenderer->lock);
	return res;
}

bool stRendererStartFrame(StRenderer *pRenderer, const f32 delta_time)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNStartFrame);

	pRenderer->view_projection = stRendererCameraMatrix(pRenderer->rvals->camera, (f32)pRenderer->rvals->width, (f32)pRenderer->rvals->height);

	//stMutexLock(&pRenderer->lock);

	if (pRenderer->state != ST_RENDERER_STATE_FRAME_PREPARE) {
		STUPID_LOG_ERROR("stRendererStartFrame() called before stRendererPrepareFrame()");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}
	else if (pRenderer->state == ST_RENDERER_STATE_FRAME_START) {
		STUPID_LOG_ERROR("stRendererStartFrame() called twice");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}
	else if (STUPID_IS_NAN(delta_time)) {
		STUPID_LOG_ERROR("stRendererStartFrame() called with NAN delta_time");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}

	stRendererFlushMap(pRenderer, &pRenderer->transformations);

	pRenderer->state = ST_RENDERER_STATE_FRAME_START;
	const bool res = pRenderer->PFNStartFrame(pRenderer->pRendererInstance, delta_time);
	//stMutexUnlock(&pRenderer->lock);
	return res;
}

bool stRendererEndFrame(StRenderer *pRenderer, const f32 delta_time)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNEndFrame);

	//stMutexLock(&pRenderer->lock);

	if (pRenderer->state != ST_RENDERER_STATE_FRAME_START) {
		STUPID_LOG_ERROR("stRendererEndFrame() called before stRendererStartFrame()");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}
	else if (pRenderer->state == ST_RENDERER_STATE_FRAME_END) {
		STUPID_LOG_ERROR("stRendererEndFrame() called twice");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}
	else if (STUPID_IS_NAN(delta_time)) {
		STUPID_LOG_ERROR("stRendererEndFrame() called with NAN delta_time");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}

	pRenderer->state = ST_RENDERER_STATE_FRAME_END;
	const bool res = pRenderer->PFNEndFrame(pRenderer->pRendererInstance, delta_time);
	pRenderer->state = ST_RENDERER_STATE_IDLE;
	//stMutexUnlock(&pRenderer->lock);
	return res;
}

bool stRendererSetClearColor(StRenderer *pRenderer, StColor color)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNSetClearColor);

	//stMutexLock(&pRenderer->lock);

	if (pRenderer->state != ST_RENDERER_STATE_FRAME_PREPARE) {
		STUPID_LOG_ERROR("stRendererSetBackgroundColor() called before stRendererPrepareFrame()");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}
	else if (STUPID_IS_NAN(color.r) || STUPID_IS_NAN(color.g) || STUPID_IS_NAN(color.b) || STUPID_IS_NAN(color.a)) {
		STUPID_LOG_ERROR("stRendererEndFrame() called with NAN color");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}

	color.r = STUPID_CLAMP(color.r, 0.0f, 1.0f);
	color.g = STUPID_CLAMP(color.g, 0.0f, 1.0f);
	color.b = STUPID_CLAMP(color.b, 0.0f, 1.0f);
	color.a = STUPID_CLAMP(color.a, 0.0f, 1.0f);

	pRenderer->PFNSetClearColor(pRenderer->pRendererInstance, color);

	//stMutexUnlock(&pRenderer->lock);

	return true;
}

bool stRendererDrawRect(StRenderer *pRenderer, i32 x, i32 y, i32 w, i32 h, const StColor color)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNDrawRect);

	//stMutexLock(&pRenderer->lock);

	if (pRenderer->state != ST_RENDERER_STATE_FRAME_START) {
		STUPID_LOG_ERROR("stRendererDrawRect() called before stRendererStartFrame()");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	StRendererValues *rvals = getRvals(pRenderer);
	if (x + w >= rvals->width) {
		i32 diff = (x + w) - (i32)rvals->width;
		w -= diff;
	}
	if (y + h >= rvals->height) {
		i32 diff = (y + h) - (i32)rvals->height;
		h -= diff;
	}
	if (w <= 0 || h <= 0) return false;

	pRenderer->PFNDrawRect(pRenderer->pRendererInstance, x, y, w, h, color);

	//stMutexUnlock(&pRenderer->lock);

	return true;
}

bool stRendererClear(StRenderer *pRenderer)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNClear);

	//stMutexLock(&pRenderer->lock);

	if (pRenderer->state != ST_RENDERER_STATE_FRAME_PREPARE) {
		STUPID_LOG_ERROR("stRendererClear() called before stRendererPrepareFrame()");
		//stMutexUnlock(&pRenderer->lock);
		return false;
	}

	pRenderer->PFNClear(pRenderer->pRendererInstance);

	//stMutexUnlock(&pRenderer->lock);

	return true;
}

StRendererValues *stRendererGetRendererValues(StRenderer *pRenderer)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->pRendererInstance);
	//stMutexLock(&pRenderer->lock);
	StRendererValues *rvals = getRvals(pRenderer);
	//stMutexUnlock(&pRenderer->lock);
	return rvals;
}

void stRendererSetVsync(StRenderer *pRenderer, const bool state)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->pRendererInstance);

	stMutexLock(&pRenderer->lock);

	pRenderer->PFNSetVsync(pRenderer->pRendererInstance, state);
	getRvals(pRenderer)->vsync = state;

	stMutexUnlock(&pRenderer->lock);
}

void stRendererCopyBufferToBuffer(StRenderer *pRenderer, const usize size, StRendererBuffer *pDestBuffer,
                                  StRendererBuffer *pSrcBuffer, const usize dest_offset, const usize src_offset)
{
	STUPID_NC(pRenderer);

	switch (pRenderer->type) {
		case ST_RENDERER_BACKEND_UNDEFINED:
		case ST_RENDERER_BACKEND_VULKAN:
			stRendererVulkanMemoryCopyBuffer(pRenderer->pRendererInstance, size, pDestBuffer->internal,
			                                 pSrcBuffer->internal, dest_offset, src_offset);
			break;
		default:
			// unreachable
			break;
	}
}

bool stRendererLoadObject(StRenderer *pRenderer, const char *path, StObject *pObject)
{
	STUPID_NC(pRenderer);
	STUPID_NC(path);
	STUPID_NC(pObject);

	fastObjMesh *mesh = fast_obj_read(path);
	STUPID_NC(mesh);

	StRendererBuffer staging_buffer;
	StRendererBuffer staging_index_buffer;

	usize size = (mesh->position_count + mesh->normal_count) * sizeof(StVec3) + mesh->texcoord_count * sizeof(
		             StVec2);
	usize index_size = mesh->index_count * sizeof(u32) * 3;

	stRendererAllocate(pRenderer, size,
	                   ST_RENDERER_BUFFER_USAGE_GENERIC | ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_SLOW |
	                   ST_RENDERER_BUFFER_USAGE_TRANSFER_SOURCE, &staging_buffer);
	stRendererAllocate(pRenderer, index_size,
	                   ST_RENDERER_BUFFER_USAGE_GENERIC | ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_SLOW |
	                   ST_RENDERER_BUFFER_USAGE_TRANSFER_SOURCE, &staging_index_buffer);

	StVec3 *map = stRendererMap(pRenderer, &staging_buffer);
	u32 *index_map = stRendererMap(pRenderer, &staging_index_buffer);

	stMemcpy(map, mesh->positions, mesh->position_count * sizeof(StVec3));
	stMemcpy(map + mesh->position_count, mesh->normals, mesh->normal_count * sizeof(StVec3));
	stMemcpy(map + mesh->position_count + mesh->normal_count, mesh->texcoords, mesh->texcoord_count * sizeof(StVec2));

	for (int i = 0; i < mesh->index_count; i++) {
		index_map[i * 3] = mesh->indices[i].p;
		index_map[i * 3 + 1] = mesh->indices[i].n + mesh->position_count;
		index_map[i * 3 + 2] = mesh->indices[i].t + mesh->position_count + mesh->normal_count;
	}

	stRendererUnmap(pRenderer, &staging_buffer);
	stRendererUnmap(pRenderer, &staging_index_buffer);

	stRendererCopyBufferToBuffer(pRenderer, size, &pRenderer->positions, &staging_buffer, pRenderer->positions_end,
	                             0);
	stRendererCopyBufferToBuffer(pRenderer, index_size, &pRenderer->indices, &staging_index_buffer,
	                             pRenderer->indices_end, 0);

	stRendererDeallocate(pRenderer, &staging_buffer);
	stRendererDeallocate(pRenderer, &staging_index_buffer);

	pObject->position_count = mesh->position_count;
	pObject->index_count = mesh->index_count;
	pObject->position_offset = pRenderer->positions_end;
	pObject->index_offset = pRenderer->indices_end;
	pObject->transformation_index = pRenderer->transformations_end;

	pRenderer->positions_end += size;
	pRenderer->indices_end += index_size;
	pRenderer->transformations_end += 1;

	fast_obj_destroy(mesh);

	return pObject;
}

void stRendererUnloadObject(StRenderer *pRenderer, StObject *pObject)
{
	pRenderer->positions_end -= pObject->position_count;
	pRenderer->indices_end -= pObject->index_count;
	stMemset(pObject, 0, sizeof(StObject));
}

bool stRendererDrawObjects(StRenderer *pRenderer, const usize count, StObject *pObjects)
{
	STUPID_NC(pRenderer);
	STUPID_NC(pRenderer->PFNDrawObjects);
	STUPID_NC(pObjects);
	STUPID_NC(pRenderer->positions.internal);
	STUPID_NC(pRenderer->indices.internal);

	STUPID_ASSERT(count < STUPID_RENDERER_MAX_OBJECTS, "object count out of bounds");

	if (pRenderer->state != ST_RENDERER_STATE_FRAME_START) {
		STUPID_LOG_ERROR("stRendererDrawObject() called before stRendererStartFrame()");
		return false;
	}

	StRendererValues *rvals = getRvals(pRenderer);

	StMat4 view = stMat4LookAt(rvals->camera.pos, rvals->camera.target, rvals->camera.up);
	StMat4 proj = stMat4Perspective(rvals->camera.fov, (f32)rvals->width / (f32)rvals->height, rvals->camera.near,
	                                rvals->camera.far);

	const StMat4 view_projection = stMat4Mul(proj, view);

	pRenderer->PFNDrawObjects(pRenderer->pRendererInstance, view_projection, count, &pRenderer->positions,
	                          &pRenderer->indices, &pRenderer->models, pObjects);

	return true;
}
