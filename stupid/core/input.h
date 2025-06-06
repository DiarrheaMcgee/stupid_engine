/// @file input.h
/// @brief Used to check keyboard, mouse, or gamepad input.
/// @note A window is required for just about everything in here.
/// @author nonexistant

#pragma once

#include "common.h"

/// Keyboard state.
typedef struct StKeyboard {
	/// State of all keys.
	bool keys[ST_KEY_MAX];
} StKeyboard;

/// Mouse state.
typedef struct StMouse {
	/// Mouse x position.
	u32 x;

	/// Mouse y position.
	u32 y;

	/// Mouse wheel position.
	u32 wheel;

	/// Mouse buttons state.
	bool buttons[MOUSE_BUTTON_MAX];
} StMouse;

/// Previous and current state of the keyboard and mouse.
typedef struct StInputState {
	/// current state of the keyboard
	StKeyboard keyboard_current;

	/// previous state of the keyboard
	StKeyboard keyboard_previous;

	/// current state of the mouse
	StMouse mouse_current;

	/// previous state of the mouse
	StMouse mouse_previous;
} StInputState;

/**
 * Creates an InputState instance.
 * @return A pointer to a new input subsystem instance.
 */
StInputState *stInputInit(void);

/**
 * Destroys an InputState instance.
 * @param pState Pointer to an InputState instance.
 */
void stInputShutdown(StInputState *pState);

/**
 * Updates an InputState struct by copying the current state to the previous state.
 * @param pState Pointer to an InputState instance.
 * @param delta_time Time since the last update.
 */
void stInputUpdate(StInputState *pState);

/**
 * Registers the state of the specified key.
 * @param pState Pointer to an InputState instance.
 * @param key Specified key.
 * @param state Whether the key is pressed.
 */
void stInputProcessKey(StInputState *pState, const st_key_id key, const bool state);

/**
 * Registers a change in the mouse position.
 * @param pState Pointer to an InputState instance.
 * @param x Mouse horizontal coordinate.
 * @param y Mouse vertical coordinate.
 */
void stInputProcessMouseMove(StInputState *pState, const u32 x, const u32 y);

/**
 * Registers the state of the specified mouse button.
 * @param pState Pointer to an InputState instance.
 * @param button Specified mouse button.
 * @param state Whether the mouse button is pressed.
 */
void stInputProcessButton(StInputState *pState, const st_mouse_button_id button, const bool state);

/**
 * Registers the state of the mouse wheel.
 * @param pState Pointer to an InputState instance.
 * @param delta The change in the mouse wheel position.
 */
void stInputProcessMouseWheel(StInputState *pState, const i8 delta);

/**
 * Checks if a key is currently pressed.
 * @param pState Pointer to an InputState instance.
 * @param key Specified key.
 * @return True if the key is currently pressed.
 */
bool stInputIsKeyPressed(const StInputState *pState, const st_key_id key);

/**
 * Checks if a key was previously pressed.
 * @param pState Pointer to an InputState instance.
 * @param key Specified key.
 * @return True if the key was previously pressed.
 */
bool stInputWasKeyPressed(const StInputState *pState, const st_key_id key);

/**
 * Checks if a mouse button is currently pressed.
 * @param pState Pointer to an InputState instance.
 * @param button Specified mouse button.
 * @return True if the mouse button is currently pressed.
 */
bool stInputIsButtonPressed(const StInputState *pState, const st_mouse_button_id button);

/**
 * Checks if a mouse button was previously pressed.
 * @param pState Pointer to an InputState instance.
 * @param button Specified mouse button.
 * @return True if the mouse button was previously pressed.
 */
bool stInputWasButtonPressed(const StInputState *pState, const st_mouse_button_id button);

/**
 * Checks if the mouse is currently moving.
 * @param pState Pointer to an InputState instance.
 * @return True if the mouse is moving.
 */
bool stInputIsMouseMoving(const StInputState *pState);

/**
 * Gets the current coordinates of the cursor.
 * @param pState Pointer to an InputState instance.
 * @param x Output horizontal coordinate.
 * @param y Output vertical coordinate.
 */
void stInputGetMousePosition(const StInputState *pState, u32 *x, u32 *y);

/**
 * Gets the previous coordinates of the cursor.
 * @param pState Pointer to an InputState instance.
 * @param x Output horizontal coordinate.
 * @param y Output vertical coordinate.
 */
void stInputGetPreviousMousePosition(const StInputState *pState, u32 *x, u32 *y);
