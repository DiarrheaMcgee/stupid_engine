#pragma once

#include "common.h"

#include "core/thread.h"
#include "core/window.h"

#include "math/linear.h"

#define STUPID_RENDERER_MAX_OBJECTS (64 * 64)
#define STUPID_RENDERER_OBJECT_POSITION_BUFFER_SIZE (256 * 1024 * 1024)
#define STUPID_RENDERER_OBJECT_INDEX_BUFFER_SIZE (256 * 1024 * 1024)

/// Color values.
typedef struct StColor {
        /// Red.
        f32 r;

        /// Green.
        f32 g;

        /// Blue.
	f32 b;

        /// Alpha.
	f32 a;
} StColor;

/// What the screen is showing stuff from.
typedef struct StCamera {
	/// Camera position.
	StVec3 pos;

	/// Point the camera is looking at.
	StVec3 target;

	/// Up direction;
	StVec3 up;

	/// Right direction;
	StVec3 right;

	/// Horizontal rotation in radians.
	f32 yaw;

	/// Vertical rotation in radians.
	f32 pitch;

	/// "Do a barrel roll."
	f32 roll;

	/// Horziontal field of view.
	f32 fov;

	/// Maximum distance for objects seen by this camera.
	f32 far;

	/// Minimum distance for objects seen by this camera.
	f32 near;
} StCamera;

/// Buffer stored in VRAM.
/// @see stRendererAllocateBuffer
typedef struct StRendererBuffer {
	/// Internal handle for the buffer.
	void *internal;

	/// Pointer used to write to the buffer.
	/// @note This is NULL unless mapped with stRendererMapBuffer.
	/// @see stRendererMapBuffer
	void *map;
} StRendererBuffer;

/// 3D Object.
typedef struct StObject {
	usize position_offset;
	usize position_count;
	usize index_offset;
	usize index_count;
	usize transformation_index;
} StObject;

/// Possible states of a StRenderer.
/// @see StRenderer
typedef enum st_renderer_state {
        /// The renderer isnt doing anything.
        ST_RENDERER_STATE_IDLE,

        /// The renderer is preparing the next frame.
        /// @see stRendererPrepareFrame
        ST_RENDERER_STATE_FRAME_PREPARE,

        /// The renderer is starting to render the frame.
        /// @see stRendererStartFrame
        ST_RENDERER_STATE_FRAME_START,

        /// The renderer is finishing the frame.
        /// @see stRendererEndFrame
        ST_RENDERER_STATE_FRAME_END,

        ST_RENDERER_STATE_MAX
} st_renderer_state;

/// StRenderer backends.
/// @see StRenderer
typedef enum st_renderer_backend {
        /// Default state (lets the renderer decide).
        ST_RENDERER_BACKEND_UNDEFINED,

        /// Vulkan (currently the only option).
        ST_RENDERER_BACKEND_VULKAN,

        ST_RENDERER_BACKEND_MAX,
} st_renderer_backend;

/// VRAM buffer usage flags.
/// @note As a general rule, more flags enabled == less performance.
/// @see StRenderer stRendererAllocateBuffer
typedef enum st_renderer_buffer_flags {
        /// Optimizes the buffer for generic storage (like matrices).
	ST_RENDERER_BUFFER_USAGE_GENERIC = 1,

	/// Optimizes the buffer for reading vertices from it.
	ST_RENDERER_BUFFER_USAGE_VERTEX = 1 << 1,

	/// Optimizes the buffer for reading indices from it.
	ST_RENDERER_BUFFER_USAGE_INDEX = 1 << 2,

	/// Optimizes the buffer for reading textures from it.
	ST_RENDERER_BUFFER_USAGE_TEXTURE = 1 << 3,

	/// Enables buffer memory mapping.
	/// @note This lowers GPU access speed.
	ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_SLOW = 1 << 4,

	/// Enables buffer memory mapping, without comprising too much on GPU access speed.
	/// @note Dont use this when you dont have to, since this is far more limited.
	ST_RENDERER_BUFFER_USAGE_CPU_ACCESS_FAST = 1 << 5,

        /// Allows the buffer to be the source of memory transfers.
	ST_RENDERER_BUFFER_USAGE_TRANSFER_SOURCE = 1 << 6
} st_renderer_buffer_flags;

/// VRAM buffer types.
/// @see StRenderer stRendererAllocateBuffer
typedef enum st_renderer_buffer_type {
        /// Does not optimize the buffer for anything in particular.
	/// @note Avoid using this when possible.
	/// @note Read only on the GPU side.
	ST_RENDERER_BUFFER_TYPE_UNDEFINED_R,

	/// Used for storing vertices.
	/// @note Read only on the GPU side.
        ST_RENDERER_BUFFER_TYPE_VERTEX_BUFFER_R,

	/// Used for storing indices (typically for vertex indices).
	/// @note Read only on the GPU side.
        ST_RENDERER_BUFFER_TYPE_INDEX_BUFFER_R,

	/// Used for storing textures.
	/// @note Read only on the GPU side.
        ST_RENDERER_BUFFER_TYPE_TEXTURE_R,

        /// Does not optimize the buffer for anything in particular.
	/// @note Avoid using this when possible.
	/// @note Writable by the GPU.
	ST_RENDERER_BUFFER_TYPE_UNDEFINED_W,

	/// Used for storing vertices.
	/// @note Avoid using this when possible.
	/// @note Writable by the GPU.
        ST_RENDERER_BUFFER_TYPE_VERTEX_BUFFER_W,

	/// Used for storing indices (typically for vertex indices).
	/// @note Writable by the GPU.
        ST_RENDERER_BUFFER_TYPE_INDEX_BUFFER_W,

	/// Used for storing textures.
	/// @note Writable by the GPU.
        ST_RENDERER_BUFFER_TYPE_TEXTURE_W,

        ST_RENDERER_BUFFER_TYPE_MAX
} st_renderer_buffer_type;

/**
 * Initializes a renderer instance.
 * @param pBackend Pointer to a renderer backend.
 * @param pWindow Window to render to.
 * @return A renderer instance.
 * @note A renderer instance relies on a renderer backend, and must be destroyed before the backend.
 * @see StRenderer
 */
typedef void *(*StPFN_renderer_init)(void *pBackend, StWindow *pWindow);

/**
 * Destroys a renderer instance.
 * @param pContext Pointer to a renderer instance.
 * @return A renderer instance.
 * @note Destroy the renderer instance before the renderer backend it is associated with.
 * @see StRenderer
 */
typedef void (*StPFN_renderer_shutdown)(void *pContext);

/**
 * Resizes a renderer instance.
 * @param pContext Pointer to a renderer instance.
 * @param width New width.
 * @param height New height.
 * @return True if successful.
 * @see StRenderer
 */
typedef bool (*StPFN_renderer_resize)(void *pContext, const u32 width, const u32 height);

/**
 * @brief Prepares a frame for rendering.
 * This puts the renderer instance in FRAME_PREPARE mode.
 * @param pContext Pointer to a renderer instance.
 * @param delta_time Time since the last frame.
 * @return True if successful.
 * @note Some functions can only be called when the renderer instance is in FRAME_PREPARE mode.
 * @see StRenderer
 */
typedef bool (*StPFN_renderer_prepare_frame)(void *pContext, const f32 delta_time);

/**
 * @brief Starts rendering a frame.
 * This puts the renderer instance in FRAME_START mode.
 * @param pContext Pointer to a renderer instance.
 * @param delta_time Time since the last frame.
 * @return True if successful.
 * @note Some functions can only be called when the renderer instance is in FRAME_START mode.
 * @see StRenderer
 */
typedef bool (*StPFN_renderer_start_frame)(void *pContext, const f32 delta_time);

/**
 * @brief Finishes rendering a frame.
 * This puts the renderer instance in FRAME_END mode.
 * @param pContext Pointer to a renderer instance.
 * @param delta_time Time since the last frame.
 * @return True if successful.
 * @note Some functions can only be called when the renderer instance is in FRAME_END mode.
 * @see StRenderer
 */
typedef bool (*StPFN_renderer_end_frame)(void *pContext, const f32 delta_time);

/**
 * Sets the clear color of a renderer instance.
 * @param pContext Pointer to a renderer instance.
 * @param color Background color.
 * @see StPFN_renderer_clear, StRenderer
 */
typedef void (*StPFN_renderer_set_clear_color)(void *pContext, const StColor color);

/**
 * Clears the background of a renderer instance.
 * @param pContext Pointer to a renderer instance.
 * @see StPFN_renderer_set_background_color, StRenderer
 */
typedef void (*StPFN_renderer_clear)(void *pContext);

/**
 * Renders a rect of the specified color.
 * @param pContext Pointer to a Vulkan renderer instance.
 * @param x Horizontal position.
 * @param y Vertical position.
 * @param w Horizontal size.
 * @param h Vertical size.
 * @param color Specified color.
 * @see StRenderer
 */
typedef void (*StPFN_renderer_draw_rect)(void *pContext, const u32 x, const u32 y, const u32 w, const u32 h, const StColor color);

/**
 * Enables or disables vsync for a renderer instance.
 * @param pContext Pointer to a renderer instance.
 * @param state The state to set vsync to.
 * @see StRenderer
 */
typedef void (*StPFN_renderer_set_vsync)(void *pContext, const bool state);

/**
 * Enables or disables vsync for a renderer instance.
 * @param pContext Pointer to a renderer instance.
 * @param pObject The object to draw.
 * @see StRenderer StObject
 */
typedef void (*StPFN_renderer_draw_objects)(void *pContext, const StMat4 view_projection, const usize object_count, StRendererBuffer *pPositionBuffer, StRendererBuffer *pIndexBuffer, StRendererBuffer *pModelBuffer, StObject *pObjects);

/**
 * Prepares the model matrices for the current frame.
 * @param pContext Pointer to a renderer instance.
 * @param model_count Number of model matrices to prepare.
 * @param pModelBuffer Output model matrix buffer.
 * @param pTransformationBuffer Properties for each object (StVec3 location, StVec3 rotation, StVec3 scale).
 * @note Optimally, this is done with a compute shader.
 * @see StRenderer StObject StMat4
 */
typedef void (*StPFN_renderer_prepare_model_matrices)(void *pContext, const usize model_count, StRendererBuffer *pModelBuffer, StRendererBuffer *pTransformationBuffer);

/// Variables found in all renderer backends.
typedef struct StRendererValues {
	/// Renderer camera.
	StCamera camera;

	/// Total frames rendered.
        u64 frames;

	/// Current width of the renderer.
	/// @note This is typically the width of the window.
	u32 width;

	/// Current height of the renderer.
	/// @note This is typically the height of the window.
	u32 height;

	/// Whether VSync is enabled or not.
        bool vsync;

	/// Whether fullscreen is enabled or not.
        bool fullscreen;
} StRendererValues;

/// StRenderer instance.
typedef struct StRenderer {
        /// Internal renderer backend instance.
        void *pRendererInstance;

        /// Init function.
        StPFN_renderer_init PFNInit;

        /// Shutdown function.
        StPFN_renderer_shutdown PFNShutdown;

        /// Resize function.
        StPFN_renderer_resize PFNResize;

        /// Frame prepare function.
        StPFN_renderer_prepare_frame PFNPrepareFrame;

        /// Frame start function.
        StPFN_renderer_start_frame PFNStartFrame;

        /// Frame end function.
        StPFN_renderer_end_frame PFNEndFrame;

        /// Set background function.
        StPFN_renderer_set_clear_color PFNSetClearColor;

        /// Background clear function.
        StPFN_renderer_clear PFNClear;

	/// Draw rect function.
	StPFN_renderer_draw_rect PFNDrawRect;

        /// Set vsync function.
        StPFN_renderer_set_vsync PFNSetVsync;

        /// Object draw function.
        StPFN_renderer_draw_objects PFNDrawObjects;

        /// Model matrix prepare function.
        StPFN_renderer_prepare_model_matrices PFNPrepareModelMatrices;

	StRendererBuffer positions;
	StRendererBuffer indices;
	StRendererBuffer models;
	StRendererBuffer transformations;

	usize position_count;

	usize positions_end;
	usize indices_end;
	usize transformations_end;

	StObject *pObjects;

        /// Renderer flags, booleans, and random values like the number of frames rendered.
        StRendererValues *rvals;

        /// Keeps things thread safe.
        StMutex lock;

        /// Renderer backend type.
        st_renderer_backend type;

        /// Current state.
        st_renderer_state state;

	StMat4 view_projection;
} StRenderer;

/// Information for a frame.
typedef struct StRendererPacket {
        /// Times since last frame.
        f32 delta;
} StRendererPacket;

