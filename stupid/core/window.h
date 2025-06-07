/// @file window.h
/// StWindow management.
/// @author nonexistant

#pragma once

#include "common.h"
//#include "input.h"

/// Maximum width allowed for a window (arbitrary).
/// @see StWindow
#define STUPID_WINDOW_MAX_WIDTH  8192u

/// Maximum height allowed for a window (arbitrary).
/// @see StWindow
#define STUPID_WINDOW_MAX_HEIGHT 8192u

/// Minimum width allowed for a window (arbitrary).
/// @see StWindow
#define STUPID_WINDOW_MIN_WIDTH  64u

/// Minimum height allowed for a window (arbitrary).
/// @see StWindow
#define STUPID_WINDOW_MIN_HEIGHT 64u

/// Disables resizing.
/// @see stWindowCreate
#define STUPID_WINDOW_NO_RESIZE 1

/// Puts the window on the center of the screen.
/// @see stWindowCreate
#define STUPID_WINDOW_CENTER (1 << 1)

/// Enables borderless mode.
/// @see stWindowCreate
#define STUPID_WINDOW_BORDERLESS (1 << 2)

/// Enables fullscreen mode.
/// @see stWindowCreate
#define STUPID_WINDOW_FULLSCREEN (1 << 3)

/// Enables borderless fullscreen mode.
/// @see stWindowCreate
#define STUPID_WINDOW_BORDERLESS_FULLSCREEN (STUPID_WINDOW_BORDERLESS | STUPID_WINDOW_FULLSCREEN)

/// Attempts to make the window invisible.
/// @see stWindowCreate
#define STUPID_WINDOW_INVISIBLE (1 << 4)

/// Attempts to force the window to be initially floating.
/// @see stWindowCreate
#define STUPID_WINDOW_FLOATING (1 << 5)

/// IDs for keyboard keys.
/// @see stInputIsKeyPressed.
typedef enum st_key_id {
	ST_KEY_UNKNOWN = 0,

	ST_KEY_A, ST_KEY_B, ST_KEY_C, ST_KEY_D, ST_KEY_E, ST_KEY_F, ST_KEY_G, ST_KEY_H, ST_KEY_I,
	ST_KEY_J, ST_KEY_K, ST_KEY_L, ST_KEY_M, ST_KEY_N, ST_KEY_O, ST_KEY_P, ST_KEY_Q, ST_KEY_R,
	ST_KEY_S, ST_KEY_T, ST_KEY_U, ST_KEY_V, ST_KEY_W, ST_KEY_X, ST_KEY_Y, ST_KEY_Z,

	ST_KEY_0, ST_KEY_1, ST_KEY_2, ST_KEY_3, ST_KEY_4, ST_KEY_5, ST_KEY_6, ST_KEY_7, ST_KEY_8, ST_KEY_9,

	ST_KEY_BACKTICK, ST_KEY_MINUS, ST_KEY_EQUALS, ST_KEY_BRACKETL,
	ST_KEY_BRACKETR, ST_KEY_BACKSLASH, ST_KEY_SEMICOLON, ST_KEY_APOSTROPHE,
	ST_KEY_COMMA, ST_KEY_PERIOD, ST_KEY_SLASH,

	ST_KEY_NUMPAD_SLASH, ST_KEY_NUMPAD_ASTERISK, ST_KEY_NUMPAD_MINUS, ST_KEY_NUMPAD_PERIOD, ST_KEY_NUMPAD_ENTER,
	ST_KEY_NUMPAD_9, ST_KEY_NUMPAD_8, ST_KEY_NUMPAD_7, ST_KEY_NUMPAD_6, ST_KEY_NUMPAD_5,
	ST_KEY_NUMPAD_4, ST_KEY_NUMPAD_3, ST_KEY_NUMPAD_2, ST_KEY_NUMPAD_1, ST_KEY_NUMPAD_0,

	ST_KEY_TAB, ST_KEY_BACKSPACE, ST_KEY_CAPSLOCK, ST_KEY_ENTER, ST_KEY_SHIFTL,
	ST_KEY_SHIFTR, ST_KEY_CONTROLL, ST_KEY_SUPERL, ST_KEY_SUPERR, ST_KEY_ALTL, ST_KEY_ALTR,
	ST_KEY_CONTROLR, ST_KEY_ESCAPE, ST_KEY_SPACE,

	ST_KEY_LEFT, ST_KEY_RIGHT, ST_KEY_DOWN, ST_KEY_UP,

	ST_KEY_F1, ST_KEY_F2, ST_KEY_F3, ST_KEY_F4, ST_KEY_F5, ST_KEY_F6,
	ST_KEY_F7, ST_KEY_F8, ST_KEY_F9, ST_KEY_F10, ST_KEY_F11, ST_KEY_F12,

	ST_KEY_PRINT, ST_KEY_SCROLLLOCK, ST_KEY_PAUSE,
	ST_KEY_INSERT, ST_KEY_HOME, ST_KEY_PAGEUP,
	ST_KEY_DELETE, ST_KEY_END, ST_KEY_PAGEDOWN,

	ST_KEY_MAX
} st_key_id;

/// Mouse button IDs.
typedef enum st_mouse_button_id {
	/// Left mouse button (MB1).
	MOUSE_BUTTON_LEFT,

	/// Right mouse button (MB2).
	MOUSE_BUTTON_RIGHT,

	/// Middle mouse button (MB3).
	MOUSE_BUTTON_MIDDLE,

	/// First side mouse button (closer to the front of the mouse).
	/// @note This is only found on some mice.
	MOUSE_BUTTON_SIDE0,

	/// Second side mouse button (closer to the back of the mouse).
	/// @note This is only found on some mice.
	MOUSE_BUTTON_SIDE1,

	MOUSE_BUTTON_MAX
} st_mouse_button_id;

/**
 * A graphical window which uses RGFW as a backend.
 * @see stWindowCreate, stWindowDestroy, Renderer
 */
typedef struct StWindow {
        /// Internal window handle.
        void *handle;

        /// Name of the video driver (i.e. wayland).
        const char *video_driver;

        /// Properties of the display.
        const void *pDisplayMode;

        /// Enabled flags.
        u32 flags;

        /// Current width.
        i32 *width;

        /// Current height.
        i32 *height;

        /// Horizontal position.
        i32 *x;

        /// Vertical position.
        i32 *y;

        /// Width before resizing.
        i32 *old_width;

        /// Height before resizing.
        i32 *old_height;

        /// Horizontal position before last move.
        i32 *old_x;

        /// Vertical position before last move.
        i32 *old_y;

        /// Maximum width.
        i32 max_width;

        /// Maximum height.
        i32 max_height;

        /// Minimum width.
        i32 min_width;

        /// Minimum height.
        i32 min_height;

	/// Center horizontal position.
	i32 center_x;

	/// Center vertical position.
	i32 center_y;

        /// Whether the window is resizable.
        bool resizable;

        /// Whether the window is currently being resized.
        bool resizing;

        /// Whether the window is fullscreen.
        bool fullscreen;

        /// Whether the window is minimized.
        bool minimized;

	/// Whether the cursor is captured.
	bool captured;
} StWindow;

/**
 * Creates a window with the specified options.
 * @param width Initial width of the window.
 * @param height Initial height of the window.
 * @param name Name of the window.
 * @param flags Window flag bitfield.
 * @return A new window which must be destroyed with stWindowDestroy().
 * @see stWindowDestroy, StWindow
 */
StWindow *(stWindowCreate)(const i32 width, const i32 height, const char *name, const u32 flags STUPID_DBG_PROTO_PARAMS);

/**
 * Creates a window with the specified options.
 * @param width Initial width of the window.
 * @param height Initial height of the window.
 * @param name Name of the window.
 * @param flags Window flag bitfield.
 * @return True if successful.
 * @see stWindowDestroy, StWindow
 */
#define stWindowCreate(width, height, name, flags) (stWindowCreate)(width, height, name, flags STUPID_DBG_PARAMS)

/**
 * Destroys a window created with stWindowCreate().
 * @param pWindow Pointer to a window to destroy.
 * @see stWindowCreate, StWindow
 */
void (stWindowDestroy)(StWindow *pWindow STUPID_DBG_PROTO_PARAMS);

/**
 * Destroys a window created with stWindowCreate().
 * @param pWindow Pointer to a window to destroy.
 * @see stWindowCreate, StWindow
 */
#define stWindowDestroy(pWindow) (stWindowDestroy)(pWindow STUPID_DBG_PARAMS)

/**
 * Resizes a window if it has resizing enabled.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @param width New width.
 * @param height New height.
 * @see StWindow
 */
void stWindowResize(StWindow *pWindow, const i32 width, const i32 height);

/**
 * Enables or disables fullscreen for a window.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @param state True to enable fullscreen, false to disable it.
 * @see StWindow
 */
void stWindowSetFullscreen(StWindow *pWindow, const bool state);

/**
 * Checks if a window is fullscreen.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @return True if the window is fullscreen.
 * @see StWindow
 */
bool stWindowIsFullscreen(StWindow *pWindow);

/**
 * Updates the state of a window, and an InputState if supplied.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @return True if successful, and the window is not supposed to close.
 * @see StWindow
 */
bool stWindowPoll(StWindow *pWindow);

/**
 * Gets the required vulkan instance extensions for a window.
 * @return stMemAlloc() array of required vulkan instance extensions.
 */
const char **stWindowGetRequiredExtensions(void);

/**
 * Creates an invisible (or hard to notice on wayland) window (this only exists for vulkan queue selection).
 * @todo Find an alternative.
 * @return A window which must be destroyed with stWindowDestroy()
 * @see stWindowDestroy, StWindow
 */
StWindow *stWindowCreateInvisible(void);

/**
 * Creates a VulkanSurface for a window.
 * @param pWindow Pointer to the window created with stWindowCreate() that the surface will be attached to.
 * @param instance A valid VkInstance.
 * @param pSurface Pointer to the output vulkan surface.
 * @return True if successful.
 * @see stWindowDestroyVulkanSurface, StVulkanSurface
 */
bool stWindowCreateVulkanSurface(StWindow *pWindow, void *instance, void *pSurface);

/**
 * Gets the current size of a window.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @param width Pointer to the output width i32.
 * @param height Pointer to the output height i32.
 * @see StWindow
 */
void stWindowGetSize(const StWindow *pWindow, i32 *width, i32 *height);

/**
 * Checks if a window is minimized.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @return True if the window is minimized.
 * @see StWindow
 */
bool stWindowIsMinimized(const StWindow *pWindow);

/**
 * Gets the refresh rate of the monitor a window was created on.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @return Refresh rate of the monitor the window was created on.
 * @see StWindow
 */
f32 stWindowGetRefreshRate(const StWindow *pWindow);

/**
 * Gets the current position of the cursor.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @param x Pointer to the output x.
 * @param y Pointer to the output y.
 */
void stWindowGetMousePosition(StWindow *pWindow, i32 *x, i32 *y);

/**
 * Checks if the cursor is currently captured by a window.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @return True if the cursor is currently captured by the specified window.
 */
bool stWindowGetCaptureCursor(const StWindow *pWindow);

/**
 * Captures or releases the cursor.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @param state True to capture the cursor, false if otherwise.
 * @note Capturing the cursor just means the cursor cant move out of the window.
 */
void stWindowSetCaptureCursor(StWindow *pWindow, const bool state);

/**
 * Checks if a key is currently pressed.
 * @param pWindow Pointer to a window created with stWindowCreate().
 * @param key Key to check.
 * @return True if the key is pressed.
 */
bool stWindowIsKeyPressed(StWindow *pWindow, st_key_id key);

/**
 * Moves the cursor to the center of the window.
 * @param pWindow Pointer to a window created with stWindowCreate().
 */
void stWindowCenterCursor(StWindow *pWindow);

/**
 * Checks vulkan present support for a physical GPU.
 * @param instance A valid VkInstance handle.
 * @param physical_device A valid VkPhysicalDevice handle.
 * @param queue_family_index Queue family to check.
 * @return True if present is supported on the queue family index.
 */
bool stWindowGetVulkanPresentationSupport(void *instance, void *physical_device, u32 queue_family_index);

