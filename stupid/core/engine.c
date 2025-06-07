/// @file engine.c
/// @brief Engine management.
/// Provides the means for initializing the engine, or shutting it down,
/// or setting up callbacks to be run on certain events like key presses.
/// This also includes the main loop.
/// @author nonexistant

#include "common.h"

#include "engine.h"
#include "clock.h"
#include "logger.h"
#include "event.h"
#include "clock.h"
#include "renderer/render_types.h"
#include "window.h"
#include "asserts.h"
#include "thread.h"

#include "memory/memory.h"

#include "renderer/render.h"

#include <signal.h>
#include <stdlib.h>

static bool signal_sent = false;

/**
 * function for shutting down all currently active subsystems
 * @param pState pointer to an engine instance
 */
static void shutdownAll(StEngine *pEngine)
{
	STUPID_NC(pEngine);

	// call the shutdown callback
	pEngine->callbackShutdown(pEngine);

	pEngine->pState->state = STUPID_ENGINE_STATE_UNINITIALIZED;

	// kill the renderer if needed
	if (pEngine->pState->pRenderer) {
		stRendererDestroy(pEngine->pState->pRenderer);
		stRendererBackendShutdown(pEngine->pState->pRendererBackend);
	}

	stWindowDestroy(pEngine->pState->pWindow);
	stEventDealloc();

	stMemDeallocNL(pEngine->pState);
}

static void signalHandler(const int sig)
{
	signal_sent = true;

	switch (sig) {
	case SIGINT:
		STUPID_LOG_INFO("received SIGINT");
		stEventFire(STUPID_EVENT_CODE_EXIT, NULL, (StEventData){0});
		break;

	case SIGABRT:
		STUPID_LOG_INFO("received SIGABRT");
		stEventFire(STUPID_EVENT_CODE_EXIT, NULL, (StEventData){0});
		break;

	case SIGTERM:
		STUPID_LOG_INFO("received SIGTERM");
		stEventFire(STUPID_EVENT_CODE_EXIT, NULL, (StEventData){0});
		break;

	case SIGHUP:
		STUPID_LOG_INFO("received SIGHUP");
		stEventFire(STUPID_EVENT_CODE_EXIT, NULL, (StEventData){0});
		break;

	// in case you want to catch SIGILL for some reason
	//case SIGILL:
	//        STUPID_LOG_CRITICAL("received SIGILL");
	//        STUPID_LOG_CRITICAL("this is probably because of a fatal error in the engine");
	//        stEventFire(STUPID_EVENT_CODE_FATAL_ERROR, (StEventData){0});
	//        exit(1);
	//        break;

	case SIGFPE:
		STUPID_LOG_CRITICAL("received SIGFPE");
		STUPID_LOG_CRITICAL("the engine divided by 0 or something (you should probably compile the engine with -O3 -ffast-math)");
		stEventFire(STUPID_EVENT_CODE_FATAL_ERROR, NULL, (StEventData){0});
		exit(1);
		break;

	case SIGSEGV:
		STUPID_LOG_CRITICAL("received SIGSEGV");
		STUPID_LOG_CRITICAL("it would seem that the code in this engine is horrible and needs to be fixed");
		stEventFire(STUPID_EVENT_CODE_FATAL_ERROR, NULL, (StEventData){0});
		exit(1);
		break;

	default:
		STUPID_LOG_CRITICAL("received unknown signal %d", sig);
		stEventFire(STUPID_EVENT_CODE_FATAL_ERROR, NULL, (StEventData){0});
		exit(1);
		break;
	}
}

/**
 * Handles most important events.
 * @param code Type of event.
 * @param sender The thing that sent this event (used to make sure something like a window resize doesnt happen to all windows).
 * @param listener The event listener (basically used to avoid static global variables).
 * @param data The data being sent in the event.
 * @return True if successful.
 */
static bool eventHandler(const st_event_code code, void *sender, void *listener, const StEventData data)
{
	STUPID_NC(listener);

	StEngine *pEngine = listener;
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);

	switch (code) {
	case STUPID_EVENT_CODE_WINDOW_RESIZED:
		pEngine->callbackResize(pEngine, *pEngine->pState->pWindow->width, *pEngine->pState->pWindow->height, data.window.w, data.window.h);
		break;

	case STUPID_EVENT_CODE_KEY_PRESSED:
		if (data.key == ST_KEY_ESCAPE && stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SHIFTR)) {
			pEngine->pState->is_suspended = !pEngine->pState->is_suspended;
			STUPID_LOG_INFO("%s", ((pEngine->pState->is_suspended) ? "engine suspended" : "engine unsuspended"));
		}
		if (!pEngine->pState->is_suspended)
			pEngine->callbackKey(pEngine, data.key, true);
		break;

	case STUPID_EVENT_CODE_EXIT:
		STUPID_LOG_INFO("quit signal received prepare to die");
		pEngine->pState->is_running = false;
		return true;
		break;

	case STUPID_EVENT_CODE_FATAL_ERROR:
		STUPID_LOG_FATAL("fatal error signal received closing immediately");
		signal_sent = true;
		stSetAtomicBoolState(&pEngine->pState->is_running, false);
		exit(1);
		break;

	case STUPID_EVENT_CODE_FPS_CHANGE:
		pEngine->pState->target_fps = data.framerate;
		pEngine->pState->target_fps_reciprocal = 1.0 / data.framerate;
		break;

	case STUPID_EVENT_CODE_KEY_RELEASED:
		if (sender != pEngine->pState->pWindow) return false;
		if (!pEngine->pState->is_suspended)
			pEngine->callbackKey(pEngine, data.key, false);
		break;

	case STUPID_EVENT_CODE_BUTTON_PRESSED:
		if (sender != pEngine->pState->pWindow) return false;
		if (!pEngine->pState->is_suspended)
			pEngine->callbackMouseButton(pEngine, data.mouse.button, true);
		break;

	case STUPID_EVENT_CODE_MOUSE_MOVED:
		if (sender != pEngine->pState->pWindow) return false;
		if (!pEngine->pState->is_suspended)
			pEngine->callbackMouseMove(pEngine, data.mouse.x, data.mouse.y);
		break;

	case STUPID_EVENT_CODE_BUTTON_RELEASED:
		if (sender != pEngine->pState->pWindow) return false;
		if (!pEngine->pState->is_suspended)
			pEngine->callbackMouseButton(pEngine, data.mouse.button, false);
		break;

	case STUPID_EVENT_CODE_FRAME_PREPARE:
		if (!pEngine->pState->is_suspended)
			pEngine->callbackFramePrepare(pEngine, data.delta);
		break;

	case STUPID_EVENT_CODE_FRAME_START:
		if (!pEngine->pState->is_suspended)
			pEngine->callbackFrameStart(pEngine, data.delta);
		break;

	case STUPID_EVENT_CODE_FRAME_END:
		if (!pEngine->pState->is_suspended)
			pEngine->callbackFrameEnd(pEngine, data.delta);
		break;

	default:
		break;
	}
	return false;
}

/// Default empty function for pEngine->callbackInitialize.
static bool placeholderInitCallback(StEngine *a) { return true; }

/// Default empty function for pEngine->callbackShutdown.
static void placeholderShutdownCallback(StEngine *a) { return; }

/// Default empty function for pEngine->callbackUpdate.
static bool placeholderUpdateCallback(StEngine *a, f32 b) { return true; }

/// Default empty function for pEngine->callbackFramePrepare.
static bool placeholderFramePrepareCallback(StEngine *a, f32 b) { return true; }

/// Default empty function for pEngine->callbackFrameStart.
static bool placeholderFrameStartCallback(StEngine *a, f32 b) { return true; }

/// Default empty function for pEngine->callbackFrameEnd.
static bool placeholderFrameEndCallback(StEngine *a, f32 b) { return true; }

/// Default empty function for pEngine->callbackResize.
static void placeholderResizeCallback(StEngine *a, i32 b, i32 c, i32 d, i32 e) { return; }

/// Default empty function for pEngine->callbackKey.
static void placeholderOnKeyCallback(StEngine *a, st_key_id b, bool c) { return; }

/// Default empty function for pEngine->callbackMouseButton.
static void placeholderOnMouseMoveCallback(StEngine *a, const i32 b, const i32 c) { return; }

/// Default empty function for pEngine->callbackMouseButton.
static void placeholderOnMouseButtonCallback(StEngine *a, st_mouse_button_id b, bool c) { return; }

st_engine_init_return_code stEngineInit(StEngine *pEngine)
{
	STUPID_NC(pEngine);

	if (pEngine->pState) {
		if (stEngineIsInitialized(pEngine)) {
			STUPID_LOG_ERROR("stEngineInit() called twice");
			return STUPID_ENGINE_INIT_CALLED_TWICE;
		}
	}

	// stack allocated clock so engine alloction time can be included in startup time
	StClock clock = {0};
	stClockStart(&clock);

	StEngineState *pEngineState = stMemAllocNL(StEngineState, 1);

	pEngineState->state = STUPID_ENGINE_STATE_UNINITIALIZED;
	pEngineState->clock = clock;

	STUPID_LOG_SYSTEM("engine started at %lf", pEngineState->clock.start_time);

#ifdef _DEBUG
	STUPID_LOG_CRITICAL("test log %lf", 1.2345);
	STUPID_LOG_FATAL("test log %lf", 1.2345);
	STUPID_LOG_ERROR("test log %lf", 1.2345);
	STUPID_LOG_WARN("test log %lf", 1.2345);
	STUPID_LOG_SYSTEM("test log %lf", 1.2345);
	STUPID_LOG_INFO("test log %lf", 1.2345);
	STUPID_LOG_DEBUG("test log %lf", 1.2345);
	STUPID_LOG_TRACE("test log %lf", 1.2345);
	STUPID_LOG_SPAM("test log     %lf", 1.2345);
	STUPID_LOG_SPAM("test log 2    %lf", 1.2345);
	STUPID_LOG_SPAM("test log %lf", 1.2345);
#endif

	STUPID_LOG_DEBUG("system clock resolution: %luNS", stGetClockResolution());

	if (!pEngine->callbackInit) {
		STUPID_LOG_WARN("init function not set");
		pEngine->callbackInit = placeholderInitCallback;
	}
	if (!pEngine->callbackShutdown) {
		STUPID_LOG_WARN("shutdown function not set");
		pEngine->callbackShutdown = placeholderShutdownCallback;
	}
	if (!pEngine->callbackUpdate) {
		STUPID_LOG_WARN("update function not set");
		pEngine->callbackUpdate = placeholderUpdateCallback;
	}
	if (!pEngine->callbackFramePrepare) {
		STUPID_LOG_WARN("frame prepare function not set");
		pEngine->callbackFramePrepare = placeholderFramePrepareCallback;
	}
	if (!pEngine->callbackFrameStart) {
		STUPID_LOG_WARN("frame start function not set");
		pEngine->callbackFrameStart = placeholderFrameStartCallback;
	}
	if (!pEngine->callbackFrameEnd) {
		STUPID_LOG_WARN("frame end function not set");
		pEngine->callbackFrameEnd = placeholderFrameEndCallback;
	}
	if (!pEngine->callbackResize) {
		STUPID_LOG_WARN("resize function not set");
		pEngine->callbackResize = placeholderResizeCallback;
	}
	if (!pEngine->callbackKey) {
		STUPID_LOG_WARN("key function not set");
		pEngine->callbackKey = placeholderOnKeyCallback;
	}
	if (!pEngine->callbackMouseButton) {
		STUPID_LOG_WARN("mouse button function not set");
		pEngine->callbackMouseButton = placeholderOnMouseButtonCallback;
	}
	if (!pEngine->callbackMouseMove) {
		STUPID_LOG_WARN("mouse move function not set");
		pEngine->callbackMouseMove = placeholderOnMouseMoveCallback;
	}

	pEngine->pState = pEngineState;
	pEngineState->is_suspended = false;

	signal(SIGINT,  signalHandler);
	//signal(SIGILL,  signalHandler);
	signal(SIGABRT, signalHandler);
	signal(SIGFPE,  signalHandler);
	signal(SIGSEGV, signalHandler);
	signal(SIGTERM, signalHandler);

	pEngineState->target_fps = (f64)pEngine->config.max_fps;

	stEventRegister(STUPID_EVENT_CODE_FATAL_ERROR,     pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_EXIT,            pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_WINDOW_RESIZED,  pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_FPS_CHANGE,      pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_KEY_PRESSED,     pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_KEY_RELEASED,    pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_MOUSE_MOVED,     pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_BUTTON_PRESSED,  pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_BUTTON_RELEASED, pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_FRAME_PREPARE,   pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_FRAME_START,     pEngine, eventHandler);
	stEventRegister(STUPID_EVENT_CODE_FRAME_END,       pEngine, eventHandler);

	pEngine->config.window.width = STUPID_CLAMP(pEngine->config.window.width, STUPID_WINDOW_MIN_WIDTH, STUPID_WINDOW_MAX_WIDTH);
	pEngine->config.window.height = STUPID_CLAMP(pEngine->config.window.height, STUPID_WINDOW_MIN_HEIGHT, STUPID_WINDOW_MAX_HEIGHT);
	pEngineState->pWindow = stWindowCreate(pEngine->config.window.width,
			                       pEngine->config.window.height,
			                       pEngine->config.name,
			                       pEngine->config.window.flags);

	if (pEngineState->pWindow == NULL) {
		STUPID_LOG_FATAL("failed to create window");
		shutdownAll(pEngine);
		return STUPID_ENGINE_INIT_WINDOW_FAILED;
	}
	if ((pEngineState->pRendererBackend = stRendererBackendInitialize(ST_RENDERER_BACKEND_VULKAN)) == NULL) {
		STUPID_LOG_FATAL("failed to initialize the stupid renderer backend");
		shutdownAll(pEngine);
		return STUPID_ENGINE_INIT_RENDERER_FAILED;
	}
	if ((pEngineState->pRenderer = stRendererCreate(pEngineState->pRendererBackend, pEngineState->pWindow)) == NULL) {
		STUPID_LOG_FATAL("failed to initialize the stupid renderer");
		shutdownAll(pEngine);
		return STUPID_ENGINE_INIT_RENDERER_FAILED;
	}

	if (!pEngine->callbackInit(pEngine)) {
		STUPID_LOG_FATAL("failed to initialize game");
		shutdownAll(pEngine);
		return STUPID_ENGINE_INIT_PFN_FAILED;
	}

	pEngine->pState = pEngineState;

	pEngine->pState->state = STUPID_ENGINE_STATE_INITIALIZED;
	pEngine->pState->tps = 60;

	stEngineSetFramerate(pEngine, pEngine->pState->target_fps);

	stClockStart(&pEngine->pState->tick_timer);
	pEngine->pState->tickrate = 1.0 / (f64)pEngine->pState->tps;

	stClockStart(&pEngine->pState->average_fps_timer);

	return STUPID_ENGINE_INIT_SUCCESS;
}

bool stEngineBeginFrame(StEngine *pEngine)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);
	
	const f32 delta = stGetClockElapsed(&pEngine->pState->clock);
	StRendererPacket packet = {0};
	packet.delta = delta;

	// TODO: fix fps lock not working correctly
	// for example if the fps lock is set to 255 the engine will run at ~252 fps
	if (pEngine->pState->fps_locked && delta < pEngine->pState->target_fps_reciprocal)
		stSleepu(STUPID_SEC_TO_US(pEngine->pState->target_fps_reciprocal - delta));

#define ALPHA 0.1
#define WAIT 1.0
	pEngine->pState->average_fps_counter += 1;
	if (stGetClockElapsed(&pEngine->pState->average_fps_timer) >= WAIT) {
		pEngine->pState->average_fps = ALPHA * pEngine->pState->average_fps + (WAIT - ALPHA) * (f64)pEngine->pState->average_fps_counter;
		stClockUpdate(&pEngine->pState->average_fps_timer);
		pEngine->pState->average_fps_counter = 0;
	}

	stClockUpdate(&pEngine->pState->clock);

	if (pEngine->pState->is_suspended) return true;
	if (!stRendererPrepareFrame(pEngine->pState->pRenderer, packet.delta)) return false;
	StEventData data = {0};
	data.delta = delta;
	stEventFire(STUPID_EVENT_CODE_FRAME_PREPARE, pEngine, data);
	if (!stRendererStartFrame(pEngine->pState->pRenderer, packet.delta)) return false;
	stEventFire(STUPID_EVENT_CODE_FRAME_START, pEngine, data);

	return true;
}

bool stEngineFinishFrame(StEngine *pEngine)
{
	STUPID_NC(pEngine);
	STUPID_NC(pEngine->pState);

	if (pEngine->pState->is_suspended) return true;
	if (!stRendererEndFrame(pEngine->pState->pRenderer, stGetClockElapsed(&pEngine->pState->clock))) return false;

	return true;
}

st_engine_shutdown_return_code stEngineShutdown(StEngine *pEngine)
{
	STUPID_NC(pEngine);

	if (pEngine->pState->state == STUPID_ENGINE_STATE_UNINITIALIZED)
		return STUPID_ENGINE_SHUTDOWN_BEFORE_INIT;

	STUPID_LOG_INFO("total frames: %lu", pEngine->pState->total_frames);

	const f64 time = pEngine->pState->clock.update_time;

	STUPID_LOG_SYSTEM("engine shutdown at %lf", stGetTime());
	shutdownAll(pEngine);
	STUPID_LOG_SYSTEM("engine shutdown took %lf", stGetTime() - time);

	return STUPID_ENGINE_SHUTDOWN_SUCCESS;
}
