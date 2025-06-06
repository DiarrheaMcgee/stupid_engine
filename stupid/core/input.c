/// @file input.c
/// @brief keyboard mouse and controller input
/// Sends events when keys or buttons are pressed, or when the mouse is moved.
/// This also provides functions for checking the position of the mouse or the status of a key or button.
/// @author nonexistant

//#include "input.h"
#include "window.h"

#include "logger.h"
#include "event.h"

#include "memory/memory.h"

/// String equivalents of st_key_id.
UNUSED static const char *key_strings[ST_KEY_MAX] = {0};

/// Mouse button names.
UNUSED static const char *button_strings[] = {
	"MB1",
	"MB3",
	"MB2",
	"MB4",
	"MB5",
	"MB6",
	"MB7",
	"MB8"
};

StInputState *stInputInit(void)
{
	StClock c = {0};
	stClockStart(&c);

	StInputState *pState = stMemAlloc(StInputState, 1);
	STUPID_NC(pState);

	STUPID_LOG_SYSTEM("input subsystem %p initialized in %lf", (void *)pState, stGetClockElapsed(&c));

	return pState;
}

void stInputShutdown(StInputState *pState)
{
	STUPID_NC(pState);

	StClock c = {0};
	stClockStart(&c);

	void *tmp = pState;
	stMemDealloc(pState);
	STUPID_LOG_SYSTEM("input subsystem %p killed in %lf", tmp, stGetClockElapsed(&c));
}

void stInputUpdate(StInputState *pState)
{
	STUPID_NC(pState);
	stMemMove(&pState->keyboard_previous, &pState->keyboard_current, sizeof(StKeyboard));
	pState->mouse_previous = pState->mouse_current;
}


void stInputProcessKey(StInputState *pState, const st_key_id key, const bool state)
{
	STUPID_NC(pState);

	// do stuff if the current state of the key is different from the previous one
	if (pState->keyboard_current.keys[key] != state) {
		pState->keyboard_current.keys[key] = state;

		StEventData data = {0};
		data.key = key;

		STUPID_ASSERT(key < ST_KEY_MAX, "key index out of bounds (what key did you just press)");

		if (state)
			stEventFire(STUPID_EVENT_CODE_KEY_PRESSED, data);
		else
			stEventFire(STUPID_EVENT_CODE_KEY_RELEASED, data);
	}
}

void stInputProcessButton(StInputState *pState, const st_mouse_button_id button, const bool state)
{
	STUPID_NC(pState);
	STUPID_ASSERT(button < MOUSE_BUTTON_MAX, "button index out of button_strings bounds (what button did you just press)");

	// do stuff if the current mouse button state is different from the previous one
	if (pState->mouse_current.buttons[button] != state) {
		pState->mouse_current.buttons[button] = state;

		StEventData data = {0};
		data.mouse.button = button;

		if (state)
			stEventFire(STUPID_EVENT_CODE_BUTTON_PRESSED, data);
		else
			stEventFire(STUPID_EVENT_CODE_BUTTON_RELEASED, data);
	}
}

void stInputProcessMouseMove(StInputState *pState, const u32 x, const u32 y)
{
	STUPID_NC(pState);

	// do stuff if the current mouse position is different from the previous one
	if (pState->mouse_current.x != x || pState->mouse_current.y != y) {
		pState->mouse_current.x = x;
		pState->mouse_current.y = y;

		StEventData data = {0};
		data.mouse.x = x;
		data.mouse.y = y;

		stEventFire(STUPID_EVENT_CODE_MOUSE_MOVED, data);
	}
}

void stInputProcessMouseWheel(StInputState *pState, const i8 delta)
{
	STUPID_NC(pState);
	StEventData data = {0};
	data.mouse.mouse_wheel = delta;
	stEventFire(STUPID_EVENT_CODE_MOUSE_WHEEL, data);
}


bool stInputIsKeyPressed(const StInputState *pState, const st_key_id key)
{
	STUPID_NC(pState);
	return pState->keyboard_current.keys[key];
}

bool stInputWasKeyPressed(const StInputState *pState, const st_key_id key)
{
	STUPID_NC(pState);
	return pState->keyboard_previous.keys[key];
}

bool stInputIsButtonPressed(const StInputState *pState, const st_mouse_button_id button)
{
	STUPID_NC(pState);
	return pState->mouse_current.buttons[button];
}

bool stInputWasButtonPressed(const StInputState *pState, const st_mouse_button_id button)
{
	STUPID_NC(pState);
	return pState->mouse_previous.buttons[button];
}

bool stInputIsMouseMoving(const StInputState *pState)
{
	STUPID_NC(pState);
	if (pState->mouse_current.x != pState->mouse_previous.x || pState->mouse_current.y != pState->mouse_previous.y)
		return true;
	else
		return false;
}

void stInputGetMousePosition(const StInputState *pState, u32 *x, u32 *y)
{
	STUPID_NC(x);
	STUPID_NC(y);
	*x = pState->mouse_current.x;
	*y = pState->mouse_current.y;
}

void stInputGetPreviousMousePosition(const StInputState *pState, u32 *x, u32 *y)
{
	STUPID_NC(x);
	STUPID_NC(y);
	*x = pState->mouse_previous.x;
	*y = pState->mouse_previous.y;
}
