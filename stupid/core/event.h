#pragma once

#include "common.h"
#include "window.h"

/// Somewhat arbitrary event code cap.
#define ST_MAX_STUPID_EVENT_CODES 512

/// Somewhat arbitrary event cap.
#define ST_MAX_EVENTS_PER_CODE 1024

/// Data passed to an event callback.
/// @see stEventFire
typedef union StEventData {
	struct {
		u32 x, y;
		u32 mouse_wheel;
		st_mouse_button_id button;
	} mouse;

	st_key_id key;

	struct {
		i32 x, y, w, h;
	} window;

	f64 framerate;
	f32 delta;

	union {
		/// @brief (void *)[2].
		/// Use this for pointers.
		void *p[2];

		/// @brief (i64)[2].
		/// Use this for large signed integers.
		i64 i64[2];

		/// @brief (u64)[2].
		/// Use this for large unsigned integers.
		u64 u64[2];

		/// @brief (f64)[2].
		/// Use this for large floats.
		f64 f64[2];

		/// @brief (i32)[4].
		/// Use this for signed integers.
		i32 i32[4];

		/// @brief (u32)[4].
		/// Use this for unsigned integers.
		u32 u32[4];

		/// @brief (f32)[4].
		/// Use this for floats.
		f32 f32[4];

		/// @brief (i16)[8].
		/// Use this for small signed integers.
		i16 i16[8];

		/// @brief (u16)[8].
		/// Use this for small unsigned integers.
		u16 u16[8];

		/// @brief (i8)[16].
		/// Use this for tiny signed integers.
		i8 i8[16];

		/// @brief (u8)[16].
		/// Use this for tiny unsigned integers.
		u8 u8[16];

		/// @brief (u8)[16].
		/// Use this for strings under 16 bytes in size.
		char c[16];
	} array;
} StEventData;

/// Pre decided event IDs.
/// @see stEventRegister stEventUnregister stEventFire
typedef enum st_event_code {
	/// Tells the engine to close on the next frame.
	STUPID_EVENT_CODE_EXIT = 0x00,

	/// Used when a key is pressed.
	STUPID_EVENT_CODE_KEY_PRESSED = 0x01,

	/// Used when a key is released.
	STUPID_EVENT_CODE_KEY_RELEASED = 0x02,

	/// Used when a mouse button is pressed.
	STUPID_EVENT_CODE_BUTTON_PRESSED = 0x03,

	/// Used when a mouse button is released.
	STUPID_EVENT_CODE_BUTTON_RELEASED = 0x04,

	/// Used when the mouse is moved.
	STUPID_EVENT_CODE_MOUSE_MOVED = 0x05,

	/// Used when the mouse wheel is moved.
	STUPID_EVENT_CODE_MOUSE_WHEEL = 0x06,

	/// Used when the target framerate is changed.
	STUPID_EVENT_CODE_FPS_CHANGE = 0x07,

	/// Used to close the engine immediately
	STUPID_EVENT_CODE_FATAL_ERROR = 0x08,

	/// Used when the next frame is being prepared.
	STUPID_EVENT_CODE_FRAME_PREPARE = 0x09,

	/// Used when the next frame is starting.
	STUPID_EVENT_CODE_FRAME_START = 0x0A,

	/// Used when the next frame is finishing.
	STUPID_EVENT_CODE_FRAME_END = 0x0B,

	/// Used when a window is resized.
	STUPID_EVENT_CODE_WINDOW_RESIZED = 0x0C,

	/// Used when a window is moved.
	STUPID_EVENT_CODE_WINDOW_MOVED = 0x0D,

	/// The maximum number of event codes.
	STUPID_EVENT_CODE_MAX = 0xFF
} st_event_code;

/**
 * Callback used when a registered event is signaled.
 * @param code Event code.
 * @param listener Pointer passed to pfn each call.
 * @param data Arguments.
 * @see stEventFire, stEventRegister
 */
typedef bool (*StPFN_event)(const st_event_code code, void *listener, const StEventData data);

/**
 * Registers a function to be called with listener passed as an argument, every time the specified event code is fired.
 * @param code Event code.
 * @param listener Pointer passed to pfn each call.
 * @param pfn Function called every time the specified event code is fired.
 * @see st_event_code, stEventUnregister, StPFN_event
 * @see stEventUnregister
 */
void (stEventRegister)(const st_event_code code, const void *listener, const StPFN_event pfn STUPID_DBG_PROTO_PARAMS);

/**
 * Registers a function to be called with listener passed as an argument, every time the specified event code is fired.
 * @param code Event code.
 * @param listener Pointer passed to pfn each call.
 * @param pfn Function called every time the specified event code is fired.
 * @see st_event_code, stEventUnregister, StPFN_event
 * @see stEventUnregister
 */
#define stEventRegister(code, listener, pfn) (stEventRegister)(code, listener, pfn STUPID_DBG_PARAMS)

/**
 * Registers a function to be called with listener passed as an argument, every time the specified event code is fired.
 * @param code Event code.
 * @param listener Pointer passed to pfn each call.
 * @param pfn Function called every time the specified event code is fired.
 * @see st_event_code, stEventUnregister, StPFN_event
 * @see stEventUnregister
 * @note Does not print logs.
 */
#define stEventRegisterNL(code, listener, pfn) (stEventRegister)(code, listener, pfn STUPID_DBG_PARAMS_NL)

/**
 * @brief Unregisters a callback registered by stEventRegister that has the same code, listener, and pfn.
 * @param code Event code.
 * @param listener Pointer passed to pfn each call.
 * @param pfn Function called every time the specified event code is fired.
 * @see stEventRegister
 */
void stEventUnregister(const st_event_code code, const void *listener, const StPFN_event pfn);

/**
 * @brief Unregisters all callbacks registered by stEventRegister with the specified code.
 * @param code Event code.
 * @see stEventRegister
 */
void stEventUnregisterCode(const st_event_code code);

/**
 * @brief Unregisters all callbacks registered by stEventRegister with the specified listener.
 * @param listener Specified listener.
 * @see stEventRegister
 */
void stEventUnregisterListener(const st_event_code code, const void *listener);

/**
 * Calls all functions registered with the specified event code and passes data to each function.
 * @param code Event code.
 * @param data Argument to pass to each function.
 */
void stEventFire(const st_event_code code, const StEventData data);

/**
 * Queues an event to be fired at the end of the current frame.
 * @param code Event code.
 * @param data Argument to pass to each function.
 */
void stEventFireLazy(const st_event_code code, const StEventData data);

/**
 * Deallocates all registered events.
 */
void stEventDealloc(void);

