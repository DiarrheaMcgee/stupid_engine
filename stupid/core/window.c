/// @file window.c
/// @brief Window management.
/// Provides the means for creating, destroying, and resizing a window.
/// @author nonexistant

#include "common.h"

#include "window.h"

#include "asserts.h"
#include "clock.h"
#include "event.h"
//#include "input.h"
#include "logger.h"

#include "memory/memory.h"

#include "renderer/vulkan/vulkan_utils.h"
#include "renderer/vulkan/vulkan_types.h"

//#include "xdg-shell.h"

#define RGFW_VULKAN
#define RGFW_NO_API
#define RGFW_IMPLEMENTATION
#define RGFW_PRINT_ERRORS
//#define RGFW_WAYLAND
//#define RGFW_NO_X11

#include <vulkan/vulkan.h>
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
//#include <vulkan/vulkan_wayland.h>

#define RGFW_ALLOC stMemAllocs
#define RGFW_FREE stMemDealloc
#define RGFW_ASSERT(x) STUPID_ASSERT(x, "RGFW internal assert")
#define RGFW_MEMCPY(dist, src, len) stMemcpy(dist, src, len)

#include "RGFW.h"

/// Lookup table for RGFW keys.
static const u16 RGFW_TO_STUPID[] = {
	[RGFW_keyNULL] = ST_KEY_UNKNOWN,

	[RGFW_a] = ST_KEY_A, [RGFW_b] = ST_KEY_B, [RGFW_c] = ST_KEY_C, [RGFW_d] = ST_KEY_D, [RGFW_e] = ST_KEY_E, [RGFW_f] = ST_KEY_F, [RGFW_g] = ST_KEY_G, [RGFW_h] = ST_KEY_H, [RGFW_i] = ST_KEY_I,
	[RGFW_j] = ST_KEY_J, [RGFW_k] = ST_KEY_K, [RGFW_l] = ST_KEY_L, [RGFW_m] = ST_KEY_M, [RGFW_n] = ST_KEY_N, [RGFW_o] = ST_KEY_O, [RGFW_p] = ST_KEY_P, [RGFW_q] = ST_KEY_Q, [RGFW_r] = ST_KEY_R,
	[RGFW_s] = ST_KEY_S, [RGFW_t] = ST_KEY_T, [RGFW_u] = ST_KEY_U, [RGFW_v] = ST_KEY_V, [RGFW_w] = ST_KEY_W, [RGFW_x] = ST_KEY_X, [RGFW_y] = ST_KEY_Y, [RGFW_z] = ST_KEY_Z,

	[RGFW_0] = ST_KEY_0, [RGFW_1] = ST_KEY_1, [RGFW_2] = ST_KEY_2, [RGFW_3] = ST_KEY_3, [RGFW_4] = ST_KEY_4, [RGFW_5] = ST_KEY_5, [RGFW_6] = ST_KEY_6, [RGFW_7] = ST_KEY_7, [RGFW_8] = ST_KEY_8, [RGFW_9] = ST_KEY_9,

	[RGFW_backtick] = ST_KEY_BACKTICK, [RGFW_minus] = ST_KEY_MINUS, [RGFW_equals] = ST_KEY_EQUALS, [RGFW_bracket] = ST_KEY_BRACKETL,
	[RGFW_closeBracket] = ST_KEY_BRACKETR, [RGFW_backSlash] = ST_KEY_BACKSLASH, [RGFW_semicolon] = ST_KEY_SEMICOLON, [RGFW_apostrophe] = ST_KEY_APOSTROPHE,
	[RGFW_comma] = ST_KEY_COMMA, [RGFW_period] = ST_KEY_PERIOD, [RGFW_slash] = ST_KEY_SLASH,

	[RGFW_KP_Slash] = ST_KEY_NUMPAD_SLASH, [RGFW_multiply] = ST_KEY_NUMPAD_ASTERISK,
	[RGFW_KP_Minus] = ST_KEY_NUMPAD_MINUS, [RGFW_KP_Period] = ST_KEY_NUMPAD_PERIOD, [RGFW_KP_Return] = ST_KEY_NUMPAD_ENTER,
	[RGFW_KP_9] = ST_KEY_NUMPAD_9, [RGFW_KP_8] = ST_KEY_NUMPAD_8, [RGFW_KP_7] = ST_KEY_NUMPAD_7, [RGFW_KP_6] = ST_KEY_NUMPAD_6, [RGFW_KP_5] = ST_KEY_NUMPAD_5,
	[RGFW_KP_4] = ST_KEY_NUMPAD_4, [RGFW_KP_3] = ST_KEY_NUMPAD_3, [RGFW_KP_2] = ST_KEY_NUMPAD_2, [RGFW_KP_1] = ST_KEY_NUMPAD_1, [RGFW_KP_0] = ST_KEY_NUMPAD_0,

	[RGFW_tab] = ST_KEY_TAB, [RGFW_backSpace] = ST_KEY_BACKSPACE, [RGFW_capsLock] = ST_KEY_CAPSLOCK, [RGFW_return] = ST_KEY_ENTER, [RGFW_shiftL] = ST_KEY_SHIFTL,
	[RGFW_shiftR] = ST_KEY_SHIFTR, [RGFW_controlL] = ST_KEY_CONTROLL, [RGFW_superL] = ST_KEY_SUPERL, [RGFW_superR] = ST_KEY_SUPERR, [RGFW_altL] = ST_KEY_ALTL, [RGFW_altR] = ST_KEY_ALTR,
	[RGFW_controlR] = ST_KEY_CONTROLR, [RGFW_escape] = ST_KEY_ESCAPE, [RGFW_space] = ST_KEY_SPACE,

	[RGFW_left] = ST_KEY_LEFT, [RGFW_right] = ST_KEY_RIGHT, [RGFW_down] = ST_KEY_DOWN, [RGFW_up] = ST_KEY_UP,

	[RGFW_F1] = ST_KEY_F1, [RGFW_F2] = ST_KEY_F2, [RGFW_F3] = ST_KEY_F3, [RGFW_F4] = ST_KEY_F4, [RGFW_F5] = ST_KEY_F5, [RGFW_F6] = ST_KEY_F6,
	[RGFW_F7] = ST_KEY_F7, [RGFW_F8] = ST_KEY_F8, [RGFW_F9] = ST_KEY_F9, [RGFW_F10] = ST_KEY_F10, [RGFW_F11] = ST_KEY_F11, [RGFW_F12] = ST_KEY_F12,

	[RGFW_scrollLock] = ST_KEY_SCROLLLOCK, [RGFW_insert] = ST_KEY_INSERT, [RGFW_home] = ST_KEY_HOME,
	[RGFW_pageUp] = ST_KEY_PAGEUP, [RGFW_pageDown] = ST_KEY_PAGEDOWN, [RGFW_delete] = ST_KEY_DELETE, [RGFW_end] = ST_KEY_END,
};

static const u16 STUPID_TO_RGFW[] = {
	[ST_KEY_UNKNOWN] = RGFW_keyNULL,

	[ST_KEY_A] = RGFW_a, [ST_KEY_B] = RGFW_b, [ST_KEY_C] = RGFW_c, [ST_KEY_D] = RGFW_d, [ST_KEY_E] = RGFW_e, [ST_KEY_F] = RGFW_f, [ST_KEY_G] = RGFW_g, [ST_KEY_H] = RGFW_h, [ST_KEY_I] = RGFW_i,
	[ST_KEY_J] = RGFW_j, [ST_KEY_K] = RGFW_k, [ST_KEY_L] = RGFW_l, [ST_KEY_M] = RGFW_m, [ST_KEY_N] = RGFW_n, [ST_KEY_O] = RGFW_o, [ST_KEY_P] = RGFW_p, [ST_KEY_Q] = RGFW_q, [ST_KEY_R] = RGFW_r,
	[ST_KEY_S] = RGFW_s, [ST_KEY_T] = RGFW_t, [ST_KEY_U] = RGFW_u, [ST_KEY_V] = RGFW_v, [ST_KEY_W] = RGFW_w, [ST_KEY_X] = RGFW_x, [ST_KEY_Y] = RGFW_y, [ST_KEY_Z] = RGFW_z,

	[ST_KEY_0] = RGFW_0, [ST_KEY_1] = RGFW_1, [ST_KEY_2] = RGFW_2, [ST_KEY_3] = RGFW_3, [ST_KEY_4] = RGFW_4, [ST_KEY_5] = RGFW_5, [ST_KEY_6] = RGFW_6, [ST_KEY_7] = RGFW_7, [ST_KEY_8] = RGFW_8, [ST_KEY_9] = RGFW_9,

	[ST_KEY_BACKTICK] = RGFW_backtick, [ST_KEY_MINUS] = RGFW_minus, [ST_KEY_EQUALS] = RGFW_equals, [ST_KEY_BRACKETL] = RGFW_bracket,
	[ST_KEY_BRACKETR] = RGFW_closeBracket, [ST_KEY_BACKSLASH] = RGFW_backSlash, [ST_KEY_SEMICOLON] = RGFW_semicolon, [ST_KEY_APOSTROPHE] = RGFW_apostrophe,
	[ST_KEY_COMMA] = RGFW_comma, [ST_KEY_PERIOD] = RGFW_period, [ST_KEY_SLASH] = RGFW_slash,

	[ST_KEY_NUMPAD_SLASH] = RGFW_KP_Slash, [ST_KEY_NUMPAD_ASTERISK] = RGFW_multiply,
	[ST_KEY_NUMPAD_MINUS] = RGFW_KP_Minus, [ST_KEY_NUMPAD_PERIOD] = RGFW_KP_Period, [ST_KEY_NUMPAD_ENTER] = RGFW_KP_Return,
	[ST_KEY_NUMPAD_9] = RGFW_KP_9, [ST_KEY_NUMPAD_8] = RGFW_KP_8, [ST_KEY_NUMPAD_7] = RGFW_KP_7, [ST_KEY_NUMPAD_6] = RGFW_KP_6, [ST_KEY_NUMPAD_5] = RGFW_KP_5,
	[ST_KEY_NUMPAD_4] = RGFW_KP_4, [ST_KEY_NUMPAD_3] = RGFW_KP_3, [ST_KEY_NUMPAD_2] = RGFW_KP_2, [ST_KEY_NUMPAD_1] = RGFW_KP_1, [ST_KEY_NUMPAD_0] = RGFW_KP_0,

	[ST_KEY_TAB] = RGFW_tab, [ST_KEY_BACKSPACE] = RGFW_backSpace, [ST_KEY_CAPSLOCK] = RGFW_capsLock, [ST_KEY_ENTER] = RGFW_return, [ST_KEY_SHIFTL] = RGFW_shiftL,
	[ST_KEY_SHIFTR] = RGFW_shiftR, [ST_KEY_CONTROLL] = RGFW_controlL, [ST_KEY_SUPERL] = RGFW_superL, [ST_KEY_SUPERR] = RGFW_superR, [ST_KEY_ALTL] = RGFW_altL, [ST_KEY_ALTR] = RGFW_altR,
	[ST_KEY_CONTROLR] = RGFW_controlR, [ST_KEY_ESCAPE] = RGFW_escape, [ST_KEY_SPACE] = RGFW_space,

	[ST_KEY_LEFT] = RGFW_left, [ST_KEY_RIGHT] = RGFW_right, [ST_KEY_DOWN] = RGFW_down, [ST_KEY_UP] = RGFW_up,

	[ST_KEY_F1] = RGFW_F1, [ST_KEY_F2] = RGFW_F2, [ST_KEY_F3] = RGFW_F3, [ST_KEY_F4] = RGFW_F4, [ST_KEY_F5] = RGFW_F5, [ST_KEY_F6] = RGFW_F6,
	[ST_KEY_F7] = RGFW_F7, [ST_KEY_F8] = RGFW_F8, [ST_KEY_F9] = RGFW_F9, [ST_KEY_F10] = RGFW_F10, [ST_KEY_F11] = RGFW_F11, [ST_KEY_F12] = RGFW_F12,

	[ST_KEY_SCROLLLOCK] = RGFW_scrollLock, [ST_KEY_INSERT] = RGFW_insert, [ST_KEY_HOME] = RGFW_home,
	[ST_KEY_PAGEUP] = RGFW_pageUp, [ST_KEY_PAGEDOWN] = RGFW_pageDown, [ST_KEY_DELETE] = RGFW_delete, [ST_KEY_END] = RGFW_end,
};


StWindow *(stWindowCreate)(i32 width, i32 height, const char *name, const u32 flags STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(name);

	StWindow *pWindow = stMemAlloc(StWindow, 1);
	StClock c = {0};
	stClockStart(&c);

	// RGFW options
	u32 internal_flags = 0;
	if (flags & STUPID_WINDOW_NO_RESIZE)  internal_flags |= RGFW_windowNoResize;
	if (flags & STUPID_WINDOW_CENTER)     internal_flags |= RGFW_windowCenter;
	if (flags & STUPID_WINDOW_BORDERLESS) internal_flags |= RGFW_windowNoBorder;
	if (flags & STUPID_WINDOW_FULLSCREEN) internal_flags |= RGFW_windowFullscreen;
	if (flags & STUPID_WINDOW_INVISIBLE)  internal_flags |= RGFW_windowHide; /* broken due to xwayland */
	if (flags & STUPID_WINDOW_FLOATING)   internal_flags |= RGFW_windowFloating;

	// clamp the width and height between the minimum and maximum
	width  = STUPID_CLAMP(width, STUPID_WINDOW_MIN_WIDTH, STUPID_WINDOW_MAX_WIDTH);
	height = STUPID_CLAMP(height, STUPID_WINDOW_MIN_HEIGHT, STUPID_WINDOW_MAX_HEIGHT);

	// create the window
	pWindow->handle = RGFW_createWindow(name, RGFW_RECT(0, 0, width, height), internal_flags);
	STUPID_NC(pWindow->handle);

	// set the maximum and mimimum dimensions
	RGFW_window_setMinSize(pWindow->handle, (RGFW_area){STUPID_WINDOW_MIN_WIDTH, STUPID_WINDOW_MIN_HEIGHT});
	RGFW_window_setMaxSize(pWindow->handle, (RGFW_area){STUPID_WINDOW_MAX_WIDTH, STUPID_WINDOW_MAX_HEIGHT});

	if (flags & STUPID_WINDOW_INVISIBLE)
		RGFW_window_move(pWindow->handle, (RGFW_point){10000, 10000});

	RGFW_window_resize(pWindow->handle, RGFW_AREA(width, height));

	// setup pointers to the rgfw window dimensions and position
	pWindow->width	    = &((RGFW_window *)pWindow->handle)->r.w;
	pWindow->height     = &((RGFW_window *)pWindow->handle)->r.h;
	pWindow->x	    = &((RGFW_window *)pWindow->handle)->r.x;
	pWindow->y	    = &((RGFW_window *)pWindow->handle)->r.y;

	// setup pointers to the rgfw old window dimensions and position
	pWindow->old_width  = &((RGFW_window *)pWindow->handle)->_oldRect.w;
	pWindow->old_height = &((RGFW_window *)pWindow->handle)->_oldRect.y;
	pWindow->old_x	    = &((RGFW_window *)pWindow->handle)->_oldRect.x;
	pWindow->old_y	    = &((RGFW_window *)pWindow->handle)->_oldRect.y;

	// set all the other window values
	pWindow->min_width  = STUPID_WINDOW_MIN_WIDTH;
	pWindow->max_width  = STUPID_WINDOW_MAX_WIDTH;
	pWindow->min_height = STUPID_WINDOW_MIN_HEIGHT;
	pWindow->max_height = STUPID_WINDOW_MAX_HEIGHT;
	pWindow->fullscreen = flags & STUPID_WINDOW_FULLSCREEN;
	pWindow->resizable  = !(flags & STUPID_WINDOW_NO_RESIZE);
	pWindow->resizing   = false;

	STUPID_LOG_TRACEFN("created window %p %dx%d", pWindow, *pWindow->width, *pWindow->height);

	return pWindow;
}

void (stWindowDestroy)(StWindow *pWindow STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(pWindow);
	STUPID_NC(pWindow->handle);

	RGFW_window_close(pWindow->handle);

	STUPID_LOG_TRACEFN("destroyed window %p", pWindow);
	stMemDealloc(pWindow);
}

void stWindowResize(StWindow *pWindow, const i32 width, const i32 height)
{
	STUPID_NC(pWindow);
	STUPID_NC(pWindow->handle);

	if (!pWindow->resizable) return;

	// set the size of the window
	RGFW_window_resize(pWindow->handle, (RGFW_area){width, height});

	StEventData data = {0};
	data.window.w = *pWindow->width;
	data.window.h = *pWindow->height;

	stEventFire(STUPID_EVENT_CODE_WINDOW_RESIZED, data);
}

void stWindowSetFullscreen(StWindow *pWindow, const bool state)
{
	STUPID_NC(pWindow);

	pWindow->resizing = true;
	RGFW_window_setFullscreen(pWindow->handle, state);
	pWindow->fullscreen = state;
	pWindow->resizing = false;
}

void stWindowGetSize(const StWindow *pWindow, i32 *width, i32 *height)
{
	STUPID_NC(pWindow);
	STUPID_NC(width);
	STUPID_NC(height);
	STUPID_NC(pWindow->width);
	STUPID_NC(pWindow->height);
	*width  = *pWindow->width;
	*height = *pWindow->height;
}

bool stWindowPoll(StWindow *pWindow)
{
	STUPID_NC(pWindow);

	RGFW_event *event = NULL;
	bool resized = false;
	
	// iterate through all available events
	while ((event = RGFW_window_checkEvent(pWindow->handle)) != NULL) {
		switch (event->type) {
		case RGFW_windowResized: {
			if (pWindow->resizing) break;
			resized = true;
			pWindow->resizing = true;
			break;
		}

		case RGFW_quit:
			return false;
			break;

		case RGFW_mousePosChanged: {
			StEventData data = {0};
			if (pWindow->captured) {
				data.mouse.x = event->vector.x;
				data.mouse.y = event->vector.y;
				stEventFire(STUPID_EVENT_CODE_MOUSE_MOVED, data);
				stWindowCenterCursor(pWindow);
			}
			else {
				data.mouse.x = event->point.x;
				data.mouse.y = event->point.x;
				stEventFire(STUPID_EVENT_CODE_MOUSE_MOVED, data);
			}
			break;
		}

		case RGFW_keyPressed: {
			StEventData data = {0};
			data.key = RGFW_TO_STUPID[event->key];
			stEventFire(STUPID_EVENT_CODE_KEY_PRESSED, data);
			break;
		}

		case RGFW_keyReleased: {
			StEventData data = {0};
			data.key = RGFW_TO_STUPID[event->key];
			stEventFire(STUPID_EVENT_CODE_KEY_RELEASED, data);
			break;
		}

		case RGFW_mouseButtonPressed: {
			StEventData data = {0};
			data.mouse.button = event->button;
			stEventFire(STUPID_EVENT_CODE_BUTTON_PRESSED, data);
			break;
		}

		case RGFW_mouseButtonReleased: {
			StEventData data = {0};
			data.mouse.button = event->button;
			stEventFire(STUPID_EVENT_CODE_BUTTON_RELEASED, data);
			break;
		}

		case RGFW_windowMoved: {
			StEventData data = {0};
			data.window.w = *pWindow->width;
			data.window.h = *pWindow->height;
			stEventFire(STUPID_EVENT_CODE_WINDOW_MOVED, data);
			break;
		}

		case RGFW_focusIn:
			if (pWindow->captured) {
				stWindowSetCaptureCursor(pWindow, true);
			}
			break;

		case RGFW_focusOut:
			RGFW_window_showMouse(pWindow->handle, RGFW_TRUE);
			break;

		default:
		       break;
		}
	}

	if (resized) {
		StEventData data = {0};
		stWindowGetSize(pWindow, &data.window.w, &data.window.h);
		stEventFire(STUPID_EVENT_CODE_WINDOW_RESIZED, data);
		pWindow->resizing = false;
	}

	pWindow->center_x = *pWindow->x + *pWindow->width / 2;
	pWindow->center_y = *pWindow->y + *pWindow->height / 2;

	return true;
}

bool stWindowIsFullscreen(StWindow *pWindow)
{
	STUPID_NC(pWindow);
	return (bool)RGFW_window_isFullscreen(pWindow->handle);
}

bool stWindowIsMinimized(const StWindow *pWindow)
{
	STUPID_NC(pWindow);
	return RGFW_window_isMinimized(pWindow->handle);
}

const char **stWindowGetRequiredExtensions(void)
{
	usize count = 0;

	const char **required_extensions = RGFW_getVKRequiredInstanceExtensions(&count);
	const char **extensions = stMemAlloc(char *, STUPID_MAX(8, count) / sizeof(char *));

	for (int i = 0; i < count; i++)
		stMemAppend(extensions, required_extensions[i]);

	return extensions;
}

bool stWindowCreateVulkanSurface(StWindow *pWindow, void *instance, void *pSurface)
{
	STUPID_NC(pWindow);
	STUPID_NC(pSurface);
	return stRendererVulkanResultIsSuccess(RGFW_window_createVKSurface(pWindow->handle, instance, pSurface));
}

StWindow *stWindowCreateInvisible(void)
{
	return stWindowCreate(64, 64, "utility window", STUPID_WINDOW_NO_RESIZE | STUPID_WINDOW_BORDERLESS | STUPID_WINDOW_INVISIBLE);
}

f32 stWindowGetRefreshRate(const StWindow *pWindow)
{
	STUPID_NC(pWindow);
	return RGFW_window_getMonitor(pWindow->handle).mode.refreshRate;
}

void stWindowGetMousePosition(StWindow *pWindow, i32 *x, i32 *y)
{
	STUPID_NC(pWindow);
	STUPID_NC(x);
	STUPID_NC(y);

	RGFW_point mouse = RGFW_window_getMousePoint(pWindow->handle);
	*x = mouse.x;
	*y = mouse.y;
}

bool stWindowGetCaptureCursor(const StWindow *pWindow)
{
	STUPID_NC(pWindow);
	return pWindow->captured;
}

void stWindowSetCaptureCursor(StWindow *pWindow, const bool state)
{
	STUPID_NC(pWindow);

	if (state) {
		int width = 0, height = 0;
		stWindowGetSize(pWindow, &width, &height);
		RGFW_window_focus(pWindow->handle);
		RGFW_window_moveMouse(pWindow->handle, RGFW_POINT(width / 2, height / 2));
		RGFW_window_mouseHold(pWindow->handle, RGFW_AREA(width, height));
		RGFW_window_showMouse(pWindow->handle, RGFW_FALSE);
	}
	else {
		RGFW_window_mouseUnhold(pWindow->handle);
		RGFW_window_showMouse(pWindow->handle, RGFW_TRUE);
	}

	pWindow->captured = state;
}

bool stWindowIsKeyPressed(StWindow *pWindow, st_key_id key)
{
	STUPID_NC(pWindow);
	STUPID_ASSERT(key >= 0 && key < ST_KEY_MAX, "key index out of bounds (what key did you just press)");

	return RGFW_isPressed(pWindow->handle, STUPID_TO_RGFW[key]);
}

void stWindowCenterCursor(StWindow *pWindow)
{
	STUPID_NC(pWindow);
	RGFW_window_moveMouse(pWindow->handle, RGFW_POINT(pWindow->center_x, pWindow->center_y));
}

bool stWindowGetVulkanPresentationSupport(void *instance, void *physical_device, u32 queue_family_index)
{
	STUPID_NC((void *)instance);
	STUPID_NC((void *)physical_device);

	if (RGFW_getVKPresentationSupport(instance, physical_device, queue_family_index) == RGFW_TRUE)
		return true;
	else
		return false;
}

