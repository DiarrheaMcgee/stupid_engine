#include <stupid/clock.h>
#include <stupid/thread.h>
#include <stupid/engine.h>
#include <stupid/logger.h>
#include <stupid/window.h>
#include <stupid/memory.h>

#include <stupid/math/sin.h>
#include <stupid/math/constants.h>

#include <stupid/renderer/render_types.h>
#include <stupid/render.h>

static StObject cube = {0};
static StObject monkey = {0};
static StObject sponza = {0};

bool handleInit(StEngine *pEngine)
{
	return true;
}

static int times_updated = 0;
bool handleUpdate(StEngine *pEngine, f64 delta_time)
{
	STUPID_LOG_INFO("update function called: %d", times_updated);
	times_updated++;
	return true;
}

static int times_rendered = 0;
bool handleRender(StEngine *pEngine, f64 delta_time)
{
	STUPID_LOG_INFO("render function called: %d", times_rendered);
	times_rendered++;
	return true;
}

void handleResize(StEngine *pEngine, const i32 old_width, const i32 old_height, const i32 width, const i32 height)
{
	STUPID_LOG_SPAM("window resized %dx%d -> %dx%d", old_width, old_height, width, height);
}

void handleKeyPress(StEngine *pEngine, const st_key_id key, const bool state)
{
	if (state)
		STUPID_LOG_SPAM("key pressed: %d", key);
	else
		STUPID_LOG_SPAM("key released: %d", key);

	if (state) {
		if (key == ST_KEY_F11)
			stWindowSetFullscreen(pEngine->pState->pWindow, !stWindowIsFullscreen(pEngine->pState->pWindow));

		else if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_CONTROLL) || stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_CONTROLR)) {
			if (key == ST_KEY_F)
				STUPID_LOG_INFO("average framerate: %f", pEngine->pState->average_fps);

			else if (key == ST_KEY_M)
				stMemUsage();

			else if (key == ST_KEY_ESCAPE && stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SHIFTR))
				pEngine->pState->is_running = false;

			else if (key == ST_KEY_L && (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_CONTROLL) || stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_CONTROLR))) {
				stEngineToggleFramerateLimit(pEngine, !stEngineIsFramerateLimited(pEngine));
				STUPID_LOG_INFO("%s", (stEngineIsFramerateLimited(pEngine) ? "fps lock enabled" : "fps lock disabled"));
			}
			else if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_CONTROLL) && stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_U)) {
				stMemUsage();
			}
			else if (key == ST_KEY_V) {
				const bool vsync_state = stRendererGetRendererValues(pEngine->pState->pRenderer)->vsync;
				stRendererSetVsync(pEngine->pState->pRenderer, !vsync_state);
				STUPID_LOG_INFO("%s", ((vsync_state) ? "vsync disabled" : "vsync enabled"));
			}
		}
		if (key == ST_KEY_TAB) {
			const bool capture_state = stWindowGetCaptureCursor(pEngine->pState->pWindow);
			stWindowSetCaptureCursor(pEngine->pState->pWindow, !capture_state);
			STUPID_LOG_INFO("%s", ((capture_state) ? "mouse capture disabled" : "mouse capture enabled"));
		}
	}
}

void handleMouseMove(StEngine *pEngine, const i32 x, const i32 y)
{
	STUPID_LOG_SPAM("mouse position: %dx%d", x, y);

	if (pEngine->pState->pWindow->captured) {
		StRendererValues *rvals = stRendererGetRendererValues(pEngine->pState->pRenderer);
		f32 yaw = x * 0.03 * pEngine->pState->tickrate;
		f32 pitch = y * 0.03 * pEngine->pState->tickrate;
		rvals->camera = stRendererCameraRotate(rvals->camera, yaw, -pitch, 0.0f);
	}
}

void handleButtonPress(StEngine *pEngine, const st_mouse_button_id button, const bool state)
{
	if (state)
		STUPID_LOG_SPAM("mouse button pressed: %d", button);
	else
		STUPID_LOG_SPAM("mouse button released: %d", button);
}

bool handleFramePrepare(StEngine *pEngine, f32 delta_time)
{
	StColor c = {0};
	c.a = 1.0f;
	c.r = 0.0;
	c.g = 0.0;
	c.b = 0.0;
	stRendererSetClearColor(pEngine->pState->pRenderer, c);
	stRendererClear(pEngine->pState->pRenderer);
	return true;
}

bool handleFrameStart(StEngine *pEngine, f32 delta_time)
{
	StObject objects[] = {monkey, cube, sponza};
	stRendererDrawObjects(pEngine->pState->pRenderer, 3, objects);

	return true;
}

int main(void)
{
	StEngine engine = {0};
	StEngine *pEngine = &engine;
	pEngine->config.window.width  = 1920;
	pEngine->config.window.height = 1080;
	pEngine->config.window.flags  = STUPID_WINDOW_CENTER | STUPID_WINDOW_FLOATING;
	pEngine->config.max_fps       = 255;
	pEngine->config.name          = "stupid engine";
	pEngine->callbackInit         = NULL;
	pEngine->callbackShutdown     = NULL;
	pEngine->callbackUpdate       = NULL;
	pEngine->callbackFramePrepare = handleFramePrepare;
	pEngine->callbackFrameStart   = handleFrameStart;
	pEngine->callbackResize       = handleResize;
	pEngine->callbackKey          = handleKeyPress;
	pEngine->callbackMouseMove    = handleMouseMove;
	pEngine->callbackMouseButton  = handleButtonPress;

	int res = 0;
	if ((res = stEngineInit(pEngine)) != STUPID_ENGINE_INIT_SUCCESS) {
		STUPID_LOG_FATAL("failed to initialize engine: %d", res);
		return 1;
	}

	STUPID_ASSERT(stRendererLoadObject(pEngine->pState->pRenderer, "assets/obj/cube.obj", &cube), "failed to load cube.obj");
	STUPID_ASSERT(stRendererLoadObject(pEngine->pState->pRenderer, "assets/obj/monkey.obj", &monkey), "failed to load monkey.obj");
	STUPID_ASSERT(stRendererLoadObject(pEngine->pState->pRenderer, "assets/obj/sponza.obj", &sponza), "failed to load sponza.obj");

	stRendererSetObjectScale(pEngine->pState->pRenderer, &cube, STVEC3(0.5, 0.5, 0.5));
	stRendererSetObjectScale(pEngine->pState->pRenderer, &monkey, STVEC3(1.2, 1.2, 1.2));
	stRendererSetObjectScale(pEngine->pState->pRenderer, &sponza, STVEC3(0.1, 0.1, 0.1));

        stRendererSetObjectTranslation(pEngine->pState->pRenderer, &monkey, STVEC3(0.0, 3.0, 0.0));

	pEngine->pState->is_running = true;

	while (stEngineIsRunning(pEngine)) {
		if (!stWindowPoll(pEngine->pState->pWindow)) break;

		if (stEngineNextTick(pEngine)) {
			StRendererValues *rvals = stRendererGetRendererValues(pEngine->pState->pRenderer);
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_W)) {
				rvals->camera = stRendererCameraMoveRelative(rvals->camera, STVEC3(0.0, 0.0, STUPID_TICKTIME(5.0, pEngine)));
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_A)) {
				rvals->camera = stRendererCameraMoveRelative(rvals->camera, STVEC3(STUPID_TICKTIME(5.0, pEngine), 0.0, 0.0));
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_S)) {
				rvals->camera = stRendererCameraMoveRelative(rvals->camera, STVEC3(0.0, 0.0, STUPID_TICKTIME(-5.0, pEngine)));
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_D)) {
				rvals->camera = stRendererCameraMoveRelative(rvals->camera, STVEC3(STUPID_TICKTIME(-5.0, pEngine), 0.0, 0.0));
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SPACE)) {
				rvals->camera = stRendererCameraMoveRelative(rvals->camera, STVEC3(0.0, STUPID_TICKTIME(5.0, pEngine), 0.0));
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SHIFTR)) {
				rvals->camera = stRendererCameraMoveRelative(rvals->camera, STVEC3(0.0, STUPID_TICKTIME(-5.0, pEngine), 0.0));
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_UP)) {
				rvals->camera = stRendererCameraRotate(rvals->camera, 0.0f, STUPID_TICKTIME(1.0, pEngine), 0.0f);
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_DOWN)) {
				rvals->camera = stRendererCameraRotate(rvals->camera, 0.0f, STUPID_TICKTIME(-1.0, pEngine), 0.0f);
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_LEFT)) {
				if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SHIFTL) || stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SHIFTR))
					rvals->camera = stRendererCameraRotate(rvals->camera, 0.0f, 0.0f, STUPID_TICKTIME(0.5, pEngine));
				else
					rvals->camera = stRendererCameraRotate(rvals->camera, STUPID_TICKTIME(-1.0, pEngine), 0.0f, 0.0f);
			}
			if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_RIGHT)) {
				if (stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SHIFTL) || stWindowIsKeyPressed(pEngine->pState->pWindow, ST_KEY_SHIFTR))
					rvals->camera = stRendererCameraRotate(rvals->camera, 0.0f, 0.0f, STUPID_TICKTIME(-0.5, pEngine));
				else
					rvals->camera = stRendererCameraRotate(rvals->camera, STUPID_TICKTIME(1.0, pEngine), 0.0f, 0.0f);
			}
		}

		STUPID_ASSERT(stEngineBeginFrame(pEngine), "failed to start frame");

		stRendererSetObjectRotation(pEngine->pState->pRenderer, &cube, STVEC3(stGetTime(), 0.0, 0.0));
		stRendererSetObjectTranslation(pEngine->pState->pRenderer, &cube, STVEC3(stCos(stGetTime()) * 2.0, stSin(stGetTime()) * 2.0 + 3.0, 0.0));

		stRendererSetObjectRotation(pEngine->pState->pRenderer, &monkey, STVEC3(0.0, stGetTime() * 0.6, 0.5));

		StObject objects[] = {monkey, cube, sponza};
		stRendererDrawObjects(pEngine->pState->pRenderer, 3, objects);

		STUPID_ASSERT(stEngineEndFrame(pEngine), "failed to finish frame");
	}

	stRendererUnloadObject(pEngine->pState->pRenderer, &cube);
	stRendererUnloadObject(pEngine->pState->pRenderer, &monkey);
	stRendererUnloadObject(pEngine->pState->pRenderer, &sponza);

	stEngineShutdown(pEngine);

	stMemUsage();

	return 0;
}
