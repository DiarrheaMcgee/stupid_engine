/// @file engine.h
/// @brief Engine instance management.
/// @author nonexistent

#pragma once

#include "stupid/common.h"
#include "stupid/clock.h"
#include "stupid/window.h"
#include "stupid/thread.h"

#include "stupid/renderer/render_types.h"

/// Enables vsync when used with engineSetFramerate().
#define STUPID_ENGINE_FRAMERATE_VSYNC ((f64)0.0)

/// Return codes for stEngineInit().
/// @see stEngineInit, StEngine
typedef enum st_engine_init_return_code {
	/// Successful initialization.
	STUPID_ENGINE_INIT_SUCCESS = 0,

	/// The user attempted to initialize the engine twice.
	STUPID_ENGINE_INIT_CALLED_TWICE = -1,

	/// Failed to create the window.
	STUPID_ENGINE_INIT_WINDOW_FAILED = -2,

	/// Failed to initialize the event subsystem.
	STUPID_ENGINE_INIT_INPUT_FAILED = -3,

	/// Failed to initialize the renderer.
	STUPID_ENGINE_INIT_RENDERER_FAILED = -4,

	/// The user supplied init callback failed.
	STUPID_ENGINE_INIT_PFN_FAILED = -5
} st_engine_init_return_code;

/// Return codes for stEngineStart().
/// @see stEngineStart, StEngine
typedef enum st_engine_start_return_code {
	/// Successfuly started the main loop.
	STUPID_ENGINE_START_SUCCESS = 0,

	/// stEngineStart() was called twice.
	STUPID_ENGINE_START_CALLED_TWICE = -1,

	/// stEngineStart() was called before stEngineInit().
	STUPID_ENGINE_START_BEFORE_INIT = -2,

	/// The user supplied update function failed.
	STUPID_ENGINE_START_PFN_UPDATE_FAILED = -3,

	/// The user supplied render function failed.
	STUPID_ENGINE_START_PFN_RENDER_FAILED = -4,

	/// Failed to start the main loop.
	STUPID_ENGINE_START_MAIN_LOOP_FAILED = -5
} st_engine_start_return_code;

/// Return codes for stEngineShutdown().
/// @see stEngineShutdown, StEngine
typedef enum st_engine_shutdown_return_code {
	STUPID_ENGINE_SHUTDOWN_SUCCESS = 0,
	STUPID_ENGINE_SHUTDOWN_BEFORE_INIT = -1
} st_engine_shutdown_return_code;

typedef enum st_engine_state {
	STUPID_ENGINE_STATE_UNINITIALIZED,
	STUPID_ENGINE_STATE_INITIALIZED,
	STUPID_ENGINE_STATE_STARTED
} st_engine_state;

/// Engine configuration.
/// @see StEngine
typedef struct StEngineConfig {
	/// Name of the program.
	char *name;

	/// Default window background colors.
	StColor default_color;

	struct {
		i32 width;
		i32 height;
		u32 flags;
	} window;

	/// Framerate limit.
	i16 max_fps;
} StEngineConfig;

typedef struct StEngine StEngine;
typedef struct StEngineState StEngineState;

/**
 * Function called on engine init.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @return True if successful.
 */
typedef bool (*StPFN_initialize)(StEngine *pEngine);

/**
 * Function called on engine shutdown.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @return True if successful.
 */
typedef void (*StPFN_shutdown)(StEngine *pEngine);

/**
 * Function called on engine update.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @param delta_time Time since the last update.
 * @return True if successful.
 */
typedef bool (*StPFN_update)(StEngine *pEngine, f32 delta_time);

/**
 * Function called on frame preparation.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @param delta_time Time since the last update.
 * @return True if successful.
 */
typedef bool (*StPFN_frame_prepare)(StEngine *pEngine, f32 delta_time);

/**
 * Function called on frame start.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @param delta_time Time since the last update.
 * @return True if successful.
 */
typedef bool (*StPFN_frame_start)(StEngine *pEngine, f32 delta_time);

/**
 * Function called on frame end.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @param delta_time Time since the last update.
 * @return True if successful.
 */
typedef bool (*StPFN_frame_end)(StEngine *pEngine, f32 delta_time);

/**
 * Function called on window resize.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @return True if successful.
 */
typedef void (*StPFN_resize)(StEngine *pEngine, i32 old_width, i32 old_height, i32 width, i32 height);

/**
 * Function called on key press/release.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @return True if successful.
 */
typedef void (*StPFN_key)(StEngine *pEngine, const st_key_id key, const bool state);

/**
 * Function called on mouse move.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @return True if successful.
 */
typedef void (*StPFN_mouse_move)(StEngine *pEngine, const i32 x, const i32 y);

/**
 * Function called on mouse button press/release.
 * @param pEngineState Pointer to an engine instance.
 * @param pEngine Pointer to a game instance.
 * @return True if successful.
 */
typedef void (*StPFN_mouse_button)(StEngine *pEngine, const st_mouse_button_id button, const bool state);

/// Instance of the entire engine.
/// @see StEngineState
typedef struct StEngine {
	/// StEngine init function.
	StPFN_initialize callbackInit;

	/// StEngine init function.
	StPFN_shutdown callbackShutdown;

	/// StEngine update function.
	StPFN_update callbackUpdate;

	/// StEngine frame prepare function.
	StPFN_frame_prepare callbackFramePrepare;

	/// StEngine frame start function.
	StPFN_frame_start callbackFrameStart;

	/// StEngine frame end function.
	StPFN_frame_end callbackFrameEnd;

	/// StEngine window resize function.
	StPFN_resize callbackResize;

	/// StEngine key press/release function.
	StPFN_key callbackKey;

	/// StEngine mouse move function.
	StPFN_mouse_move callbackMouseMove;

	/// StEngine mouse button press/release function.
	StPFN_mouse_button callbackMouseButton;

	/// Internal engine state.
	StEngineState *pState;

	/// User supplied internal state.
	void *pUserState;

	/// Engine configuration.
	StEngineConfig config;
} StEngine;

/// State of an engine instance.
/// @see StEngine
typedef struct StEngineState {
	/// Keeps track of time in the engine.
	StClock clock;

	/// Time the engine has been running.
	f64 running_time;

	/// Target framerate.
	f64 target_fps;

	/// 1 / target framerate.
	f64 target_fps_reciprocal;

	/// Average framerate over the last 3 seconds.
	f64 average_fps;

	/// Used to keep track of the average framerate.
	StClock average_fps_timer;

	/// Used to keep track of how many frames have passed since updating the average framerate.
	u32 average_fps_counter;

	f64 last_tick_time;

	/// Total frames rendered.
	/// @note This does not increment while the engine is suspended.
	u64 total_frames;

	/// Main window.
	StWindow *pWindow;

	/// Renderer backend instance.
	void *pRendererBackend;

	/// Renderer instance.
	StRenderer *pRenderer;

	/// If the engine is currently running.
	STUPID_ATOMIC bool is_running;

	/// If the main loop is currently running.
	STUPID_ATOMIC bool main_loop_is_running;

	/// If the engine is suspended then the main loop will be paused.
	STUPID_ATOMIC bool is_suspended;

	/// Pretty self explanatory.
	STUPID_ATOMIC bool is_minimized;

	/// Whether the framerate should be limited to target_framerate.
	STUPID_ATOMIC bool fps_locked;

	/// Keeps things (theoretically) thread safe.
	StMutex lock;

	/// The current state (like STUPID_ENGINE_STATE_INITIALIZED or something).
	st_engine_state state;

	/// Ticks queued.
	u16 ticks;

	/// @brief Used to keep track of time, and how many ticks are queued.
	/// For example, if the time between ticks is 0.1, and tick_time is 0.2,
	/// one tick will happen, and the timer will be reduced to 0.1.
	f64 tick_time;

	StClock tick_timer;

	/// Ticks per second.
	u16 tps;

	/// Time between ticks.
	f64 tickrate;
} StEngineState;

/**
 * Initializes an engine instance.
 * @param pEngineInstance Pointer to an engine instance.
 * @return An engine init return code.
 * @see stEngineStart, stEngineShutdown, st_engine_init_return_code
 */
st_engine_init_return_code stEngineInit(StEngine *pEngine);

/**
 * Starts an existing engine instance.
 * @param pEngine Pointer to an engine instance state.
 * @return An engine start return code.
 * @see stEngineStop, stEngineInit, st_engine_start_return_code
 */
st_engine_start_return_code stEngineStart(StEngine *pEngine);

/**
 * Shuts down an engine instnace.
 * @param pEngine Pointer to an engine instance.
 * @return An engine shutdown return code.
 * @see stEngineStop, stEngineInit, st_engine_shutdown_return_code
 */
st_engine_shutdown_return_code stEngineShutdown(StEngine *pState);

bool stEngineBeginFrame(StEngine *pEngine);
bool stEngineEndFrame(StEngine *pEngine);

/**
 * Checks if an engine instance is currently running.
 * @param pEngine Pointer to an engine instance.
 * @return True if the specified engine instance is running.
 */
static STUPID_INLINE bool stEngineIsRunning(const StEngine *pEngine)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	stMutexLock(&pEngine->pState->lock);
	const bool res = pEngine->pState->is_running;
	stMutexUnlock(&pEngine->pState->lock);
	return res;
}

/**
 * Checks if an engine instance is currently running.
 * @param pEngine Pointer to an engine instance.
 * @return True if the specified engine instance is running.
 */
static STUPID_INLINE bool stEngineNextFrame(const StEngine *pEngine)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	if (!stEngineIsRunning(pEngine)) return false;
	return true;
}

static STUPID_INLINE bool stEngineNextTick(StEngine *pEngine)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	//stFenceReset(&pEngine->pState->frame_start);

	pEngine->pState->tick_time += stGetClockElapsed(&pEngine->pState->tick_timer);
	stClockUpdate(&pEngine->pState->tick_timer);

	if (pEngine->pState->tick_time >= pEngine->pState->tickrate) {
		pEngine->pState->tick_time -= pEngine->pState->tickrate;
		return true;
	}
	else
		return false;

	//pEngine->pState->frame_start.lock = false;
	//pEngine->pState->frame_start.lock = true;
	//stFenceSignal(&pEngine->pState->frame_start);
}

#define STUPID_TICKTIME(n, pEngine) ((f64)(n) / (f64)(pEngine)->pState->tps)

/**
 * Checks if an engine instance is initialized.
 * @param pEngine Pointer to an engine instance.
 * @return True if the specified engine instance is initialized.
 */
static STUPID_INLINE bool stEngineIsInitialized(const StEngine *pEngine)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	const bool res = (pEngine->pState->state == STUPID_ENGINE_STATE_INITIALIZED || pEngine->pState->state == STUPID_ENGINE_STATE_STARTED);
	stMutexUnlock(&pEngine->pState->lock);
	return res;
}

/**
 * Sets the framerate cap of an engine instance.
 * @param pEngine Pointer to an engine instance.
 * @param fps Framerate limit.
 */
static STUPID_INLINE void stEngineSetFramerate(StEngine *pEngine, const f64 fps)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	stMutexLock(&pEngine->pState->lock);
	if (fps == STUPID_ENGINE_FRAMERATE_VSYNC) pEngine->pState->target_fps_reciprocal = 1.0 / STUPID_MAX(stWindowGetRefreshRate(pEngine->pState->pWindow), 60.0);
	else pEngine->pState->target_fps_reciprocal = 1.0 / STUPID_MAX(fps, 60.0);
	stMutexUnlock(&pEngine->pState->lock);
}


/**
 * Checks if the framerate of an engine instance is capped.
 * @param pEngine Pointer to an engine instance.
 */
static STUPID_INLINE bool stEngineIsFramerateLimited(const StEngine *pEngine)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	stMutexLock(&pEngine->pState->lock);
	const bool res = pEngine->pState->fps_locked;
	stMutexUnlock(&pEngine->pState->lock);
	return res;
}

/**
 * Toggles the framerate limit of the engine.
 * @param pEngine Pointer to an engine instance.
 * @param state True to cap the framerate, false to uncap it.
 */
static STUPID_INLINE void stEngineToggleFramerateLimit(StEngine *pEngine, const bool state)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	stMutexLock(&pEngine->pState->lock);
	pEngine->pState->fps_locked = state;
	stMutexUnlock(&pEngine->pState->lock);
}
