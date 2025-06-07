#pragma once

#include "stupid/common.h"
#include "stupid/logger.h"

#include "stupid/math/linear.h"

#include "stupid/renderer/render_types.h"

/**
 * Initializes a renderer backend.
 * @param backend Which backend to initialize.
 * @return The requested renderer backend.
 * @note Dont create more than one vulkan backend instance.
 * @note At the moment, only vulkan is available.
 * @see stRendererBackendShutdown
 */
void *stRendererBackendInitialize(const st_renderer_backend backend);

/**
 * Shuts down a renderer backend.
 * @param pBackend A backend created with stRendererInitializeBackend()
 */
void stRendererBackendShutdown(void *pBackend);

/**
 * Creates a renderer instance.
 * @param pBackend A backend created with stRendererInitializeBackend()
 * @param pWindow Pointer to a window.
 * @return A new renderer instance which must be destroyed with stRendererDestroy().
 * @see stRendererDestroy, Renderer
 */
StRenderer *stRendererCreate(void *pBackend, StWindow *pWindow);

/**
 * Destroys a renderer instance.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @see stRendererCreate, Renderer
 */
void stRendererDestroy(StRenderer *pRenderer);

/**
 * Allocates a buffer in VRAM.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param size Number of bytes to allocate.
 * @param type Type of buffer to allocate.
 * @param pBuffer Output buffer.
 * @see StRenderer StBuffer stRendererDeallocate
 */
void (stRendererAllocate)(StRenderer *pRenderer, const usize size, const st_renderer_buffer_flags flags, StRendererBuffer *pBuffer STUPID_DBG_PROTO_PARAMS);

/**
 * Allocates a buffer in VRAM.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param size Number of bytes to allocate.
 * @param type Type of buffer to allocate.
 * @param pBuffer Output buffer.
 * @see StRenderer StBuffer stRendererDeallocate
 */
#define stRendererAllocate(pRenderer, size, flags, pBuffer) (stRendererAllocate)(pRenderer, size, flags, pBuffer STUPID_DBG_PARAMS);

/**
 * Deallocates a StRendererBuffer.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pBuffer Buffer to deallocate.
 * @see StRenderer StBuffer stRendererAllocate
 */
void (stRendererDeallocate)(StRenderer *pRenderer, StRendererBuffer *pBuffer STUPID_DBG_PROTO_PARAMS);

/**
 * Deallocates a StRendererBuffer.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pBuffer Buffer to deallocate.
 * @see StRenderer StBuffer stRendererAllocate
 */
#define stRendererDeallocate(pRenderer, pBuffer) (stRendererDeallocate)(pRenderer, pBuffer STUPID_DBG_PARAMS);

/**
 * Maps a StRendererBuffer to CPU accessible memory.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pBuffer Buffer to map.
 * @return Address the buffer was mapped to.
 * @note This just means that the pointer its mapped to can be used to access the buffer.
 * @see StRenderer StBuffer stRendererUnmap
 */
void *stRendererMap(StRenderer *pRenderer, StRendererBuffer *pBuffer);

/**
 * Unmaps a StRendererBuffer from CPU accessible memory.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pBuffer Buffer to unmap.
 * @see StRenderer StBuffer stRendererMap
 */
void stRendererUnmap(StRenderer *pRenderer, StRendererBuffer *pBuffer);

/**
 * Makes sure all the memory transfers to the buffer have finished.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pBuffer Buffer to flush.
 * @see StRenderer StBuffer stRendererMap
 */
void stRendererFlushMap(StRenderer *pRenderer, StRendererBuffer *pBuffer);

/**
 * Resizes a renderer instance.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param width New width.
 * @param height New height.
 * @return True if successful.
 * @see Renderer
 */
bool stRendererResize(StRenderer *pRenderer, const i32 width, const i32 height);

/**
 * Prepares the next frame.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param delta_time Time since the last frame.
 * @see stRendererStartFrame
 */
bool stRendererPrepareFrame(StRenderer *pRenderer, f32 delta_time);

/**
 * Starts the next frame.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param delta_time Time since the last frame.
 * @see stRendererEndFrame
 */
bool stRendererStartFrame(StRenderer *pRenderer, const f32 delta_time);

/**
 * Prepares the current frame.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param delta_time Time since the last frame.
 * @see stRendererStartFrame
 */
bool stRendererEndFrame(StRenderer *pRenderer, const f32 delta_time);

/**
 * Sets the clear color of a renderer instance.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param color Clear color.
 * @see stRendererClear
 */
bool stRendererSetClearColor(StRenderer *pRenderer, StColor color);

/**
 * Renders a rect of the specified color.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param x Horizontal position.
 * @param y Vertical position.
 * @param w Horizontal size.
 * @param h Vertical size.
 * @param color Specified color.
 * @see stRendererDrawRect
 */
bool stRendererDrawRect(StRenderer *pRenderer, i32 x, i32 y, i32 w, i32 h, const StColor color);

/**
 * Clears a renderer.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @return True if successful.
 * @note If the frame isnt cleared then it defaults to a bunch of jumbled up garbage.
 * @see stRendererSetClearColor
 */
bool stRendererClear(StRenderer *pRenderer);

/**
 * Enables or disables vsync for a renderer instance.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param state True to enable vsync, false to disable it.
 */
void stRendererSetVsync(StRenderer *pRenderer, const bool state);

/**
 * Gets a pointer to the renderers internal variables (such as width height and vsync).
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @return A pointer to the internal renderer backends StRendererValues struct if successful, NULL otherwise.
 * @note These are the same across backends.
 * @see StRendererValues
 */
StRendererValues *stRendererGetRendererValues(StRenderer *pRenderer);

/**
 * Loads an OBJ file.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pObject Output StObject.
 * @return True if successful.
 * @see stRendererUnloadObject.
 */
bool stRendererLoadObject(StRenderer *pRenderer, const char *path, StObject *pObject);

/**
 * Unloads an OBJ file.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pObject Object to unload.
 * @return True if successful.
 * @see stRendererLoadObject.
 */
void stRendererUnloadObject(StRenderer *pRenderer, StObject *pObject);

/**
 * Draws an object to the screen.
 * @param pRenderer Pointer to a renderer instance created with stRendererCreate().
 * @param pObject Object to draw.
 * @return True if successful.
 * @see stRendererLoadObject.
 */
bool stRendererDrawObjects(StRenderer *pRenderer, const usize count, StObject *pObjects);

static STUPID_INLINE StCamera stRendererCameraMove(StCamera camera, StVec3 offset)
{
	camera.pos = stVec3Add(camera.pos, offset);
	camera.target = stVec3Add(camera.target, offset);
	return camera;
}

static STUPID_INLINE StCamera stRendererCameraCreate(const StVec3 position, const StVec3 target, const f32 fov, const f32 near, const f32 far)
{
	StCamera camera = {0};
	camera.pos = position;
	camera.target = target;
	if (stVec3Distance(camera.pos, camera.target) <= STUPID_F32_EPSILON)
		camera.target = stVec3Add(camera.target, STVEC3(0.0, 0.0, -1.0));
	camera.fov   = fov;
	camera.near  = near;
	camera.far   = far;

	StVec3 d = stVec3Normalize(stVec3Sub(camera.target, camera.pos));

	camera.yaw   = stAtan2(d.y, d.x);
	camera.pitch = stAtan2(d.z, stSqrt(d.x * d.x + d.y * d.y));
	camera.roll  = 0.0;

	StVec3 world_up = STVEC3(0.0, -1.0, 0.0);
		
	camera.right = stVec3Cross(d, world_up);
	camera.up = stVec3Cross(d, camera.right);

	return camera;
}

static STUPID_INLINE StMat4 stRendererCameraMatrix(const StCamera camera, const f32 width, const f32 height)
{
	const StMat4 view = stMat4LookAt(camera.pos, camera.target, camera.up);
	const StMat4 proj = stMat4Perspective(camera.fov, (f32)width / (f32)height, camera.near, camera.far);
	return stMat4Mul(proj, view);
}

static STUPID_INLINE StCamera stRendererCameraRotate(StCamera camera, const f32 yaw, const f32 pitch, const f32 roll)
{
	camera.yaw += yaw;
	camera.pitch += pitch;
	camera.roll += roll;

	if (STUPID_UNLIKELY(camera.pitch > STUPID_MATH_TAUd4 - 0.001))
		camera.pitch = STUPID_MATH_TAUd4 - 0.001;
	if (STUPID_UNLIKELY(camera.pitch < -STUPID_MATH_TAUd4 + 0.001))
		camera.pitch = -STUPID_MATH_TAUd4 + 0.001;

	if (STUPID_UNLIKELY(camera.yaw > STUPID_MATH_TAU))
		camera.yaw -= STUPID_MATH_TAU;
	if (STUPID_UNLIKELY(camera.yaw < -STUPID_MATH_TAU))
		camera.yaw += STUPID_MATH_TAU;

	StVec3 direction = {0};
	direction.x = -stSin(camera.yaw) * stCos(camera.pitch);
	direction.y = stSin(camera.pitch);
	direction.z = -stCos(camera.yaw) * stCos(camera.pitch);

	f32 distance = stVec3Hypot(stVec3Sub(camera.target, camera.pos));
	camera.target = stVec3Add(camera.pos, stVec3Scale(direction, distance));

	StVec3 world_up = STVEC3(0.0, -1.0, 0.0);

	// TODO: fix broken roll
	camera.right = stVec3MulMat(stVec3Cross(direction, world_up), stMat3Rotate(camera.roll, direction));
	camera.up = stVec3Cross(direction, camera.right);

	return camera;
}

static STUPID_INLINE StCamera stRendererCameraMoveRelative(StCamera camera, StVec3 offset)
{
	StVec3 forward = stVec3Sub(camera.target, camera.pos);
	const f32 distance = stVec3Hypot(forward);

	if (distance < 0.00001)
		return camera;

	forward = stVec3Normalize(forward);

	const StVec3 right = stVec3Normalize(stVec3Cross(forward, camera.up));
	const StVec3 up = stVec3Normalize(stVec3Cross(right, forward));

	StVec3 world_offset = {0};
	world_offset.x = (offset.x * right.x) + (offset.y * up.x) + (offset.z * forward.x);
	world_offset.y = (offset.x * right.y) + (offset.y * up.y) + (offset.z * forward.y);
	world_offset.z = (offset.x * right.z) + (offset.y * up.z) + (offset.z * forward.z);

	return stRendererCameraMove(camera, world_offset);
}

static STUPID_INLINE void stRendererSetObjectTranslation(StRenderer *pRenderer, StObject *pObject, StVec3 position)
{
	STUPID_NC(pRenderer->transformations.map);
	StVec3 *map = pRenderer->transformations.map;
	map[pObject->transformation_index * 3] = position;
}

static STUPID_INLINE void stRendererSetObjectRotation(StRenderer *pRenderer, StObject *pObject, StVec3 rotation)
{
	STUPID_NC(pRenderer->transformations.map);
	StVec3 *map = pRenderer->transformations.map;
	map[pObject->transformation_index * 3 + 1] = rotation;
}

static STUPID_INLINE void stRendererSetObjectScale(StRenderer *pRenderer, StObject *pObject, StVec3 scale)
{
	STUPID_NC(pRenderer->transformations.map);
	StVec3 *map = pRenderer->transformations.map;
	map[pObject->transformation_index * 3 + 2] = scale;
}

