#pragma once

#include "common.h"

// disable trace and debug logs if compiled in release mode (unless otherwise specified)
#ifndef _DEBUG
	#ifndef STUPID_LOG_DEBUG_ENABLED
		#define STUPID_LOG_DEBUG_DISABLED
	#endif

	#ifndef STUPID_LOG_TRACE_ENABLED
		#define STUPID_LOG_TRACE_DISABLED
	#endif
#endif // _DEBUG

/// Foreground colors for stLogCustom().
typedef enum st_text_color {
	/// White text.
	ST_TEXT_COLOR_WHITE,

	/// Red text.
	ST_TEXT_COLOR_RED,

	/// Green text.
	ST_TEXT_COLOR_GREEN,

	/// Yellow text.
	ST_TEXT_COLOR_YELLOW,

	/// Blue text.
	ST_TEXT_COLOR_BLUE,

	/// Purple text.
	ST_TEXT_COLOR_PURPLE,

	/// Cyan text.
	ST_TEXT_COLOR_CYAN,

	/// Black text.
	ST_TEXT_COLOR_BLACK,

	ST_TEXT_COLOR_MAX
} st_text_color;

/// Background colors for stLogCustom().
typedef enum st_text_background_color {
	/// Blank background.
	ST_TEXT_BACKGROUND_NONE,

	/// White background.
	ST_TEXT_BACKGROUND_WHITE,

	/// Red background.
	ST_TEXT_BACKGROUND_RED,

	/// Green background.
	ST_TEXT_BACKGROUND_GREEN,

	/// Yellow background.
	ST_TEXT_BACKGROUND_YELLOW,

	/// Blue background.
	ST_TEXT_BACKGROUND_BLUE,

	/// Purple background.
	ST_TEXT_BACKGROUND_PURPLE,

	/// Cyan background.
	ST_TEXT_BACKGROUND_CYAN,

	/// Black background.
	ST_TEXT_BACKGROUND_BLACK,

	ST_TEXT_BACKGROUND_MAX
} st_text_background_color;

/// Special properties for stLogCustom().
typedef enum st_text_property {
	/// Bold text.
	ST_TEXT_PROPERTY_BOLD = 1,

	/// Italic text.
	ST_TEXT_PROPERTY_ITALIC = 1 << 1,

	/// Underline text.
	ST_TEXT_PROPERTY_UNDERLINE = 1 << 2,

	/// Bright foreground.
	ST_TEXT_PROPERTY_BRIGHT = 1 << 3,

	/// Bright background.
	ST_TEXT_PROPERTY_BRIGHT_BACKGROUND = 1 << 4,

	/// Dim foreground.
	ST_TEXT_PROPERTY_DIM = 1 << 5,

	/// Insert a linebreak at the end.
	ST_TEXT_PROPERTY_LINEBREAK = 1 << 6,

	/// Replace this log with the next log.
	ST_TEXT_PROPERTY_BACKSPACE = 1 << 7,

	/// Send the message immediately (useful for things like mouse position logs).
	ST_TEXT_PROPERTY_FLUSH = 1 << 8,

	/// Use stderr.
	ST_TEXT_PROPERTY_ERR = 1 << 9,

	ST_TEXT_PROPERTY_MAX = 1 << 10
} st_text_property;

/// Log severities.
typedef enum st_log_level {
	/// Critical errors like segfaults.
	ST_LOG_LEVEL_CRITICAL,

	/// Fatal errors like renderer backend errors.
	ST_LOG_LEVEL_FATAL,

	/// Regular errors like invalid parameters on less important functions.
	ST_LOG_LEVEL_ERROR,

	/// Warnings about subpar situations.
	ST_LOG_LEVEL_WARNING,

	/// Generally used to log creation and destruction of subsystems.
	ST_LOG_LEVEL_SYSTEM,

	/// Used to log basically everything else.
	ST_LOG_LEVEL_INFO,

	/// Debug only info logs.
	ST_LOG_LEVEL_DEBUG,

	/// These logs are replaced by the next log so its useful for spammy things like printing the mouse position.
	ST_LOG_LEVEL_SPAM,

	/// Debug only trace logs for things like memory allocation and thread creation.
	ST_LOG_LEVEL_TRACE,

	ST_LOG_LEVEL_MAX
} st_log_level;

/**
 * Prints text with the specified properties.
 * @param message String to print.
 * @param color Text foreground color.
 * @param background_color Text background color.
 * @param properties Text special properties.
 * @param ... printf format arguments.
 */
int stLogCustom(const st_text_color color, const st_text_background_color background_color, const st_text_property properties, const char *message, ...) STUPID_ATTR_FORMAT(4, 5);

/**
 * Logs a message with the specified log level.
 * @param level The log severity.
 * @param message String to print.
 * @param ... printf format arguments.
 */
void stLog(const st_log_level level, const char *message, ...) STUPID_ATTR_FORMAT(2, 3);

/**
 * @brief Prints a trace log.
 * Prints the name of the function this was called from, the filename, the line, and the message.
 * @param fn name of the function
 * @param file name of the file
 * @param line line number
 * @param message string to print
 * @param ... printf format arguments
 */
void stLogTrace(const char* fn, const char *file, const int line, const char *message, ...) STUPID_ATTR_FORMAT(4, 5);

/// This just gets rid of unused variable errors for variable arguments.
static STUPID_INLINE void _stLoggerStopUnusedArgumentWarning(const char *message, ...) { return; }

#ifndef STUPID_LOG_CRITICAL_DISABLED
/**
 * print a critical error log
 * @param message printf format string
 */
#define STUPID_LOG_CRITICAL(message, ...) stLog(ST_LOG_LEVEL_CRITICAL, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_CRITICAL(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif // STUPID_LOG_SPAM_ENABLED

#ifndef STUPID_LOG_FATAL_DISABLED
/**
 * print a fatal error log
 * @param message printf format string
 */
#define STUPID_LOG_FATAL(message, ...) stLog(ST_LOG_LEVEL_FATAL, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_FATAL(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif // STUPID_LOG_FATAL_ENABLED

#ifndef STUPID_LOG_ERROR_DISABLED
/**
 * prints an error log
 * @param message printf format string
 */
#define STUPID_LOG_ERROR(message, ...) stLog(ST_LOG_LEVEL_ERROR, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_ERROR(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif // STUPID_LOG_ERROR_ENABLED

#ifndef STUPID_LOG_WARN_DISABLED
/**
 * prints a warning log
 * @param message printf format string
 */
#define STUPID_LOG_WARN(message, ...) stLog(ST_LOG_LEVEL_WARNING, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_WARN(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif // STUPID_LOG_WARN_ENABLED

#ifndef STUPID_LOG_INFO_DISABLED
/**
 * prints an info log
 * @param message printf format string
 */
#define STUPID_LOG_INFO(message, ...) stLog(ST_LOG_LEVEL_INFO, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_INFO(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif

#ifndef STUPID_LOG_SYSTEM_DISABLED
/**
 * prints a system log (usually for subsystem initialization)
 * @param message printf format string
 */
#define STUPID_LOG_SYSTEM(message, ...) stLog(ST_LOG_LEVEL_SYSTEM, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_SYSTEM(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif

#ifndef STUPID_LOG_SPAM_DISABLED
/**
 * prints a spam log that gets overwritten by the next log
 * @param message printf format string
 */
#define STUPID_LOG_SPAM(message, ...) stLog(ST_LOG_LEVEL_SPAM, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_SPAM(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif // STUPID_LOG_SPAM_ENABLED

#ifndef STUPID_LOG_DEBUG_DISABLED
/**
 * prints a debug log
 * @param message printf format string
 */
#define STUPID_LOG_DEBUG(message, ...) stLog(ST_LOG_LEVEL_DEBUG, "%s(): " message, __FUNCTION__, ##__VA_ARGS__)
#else
#define STUPID_LOG_DEBUG(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#endif // STUPID_LOG_DEBUG_ENABLED

#ifndef STUPID_LOG_TRACE_DISABLED
/**
 * prints a trace log (usually for things like memory allocation)
 * @param message printf format string
 */
#define STUPID_LOG_TRACE(message, ...) STUPID_DBG(stLog(ST_LOG_LEVEL_TRACE, "%s(): " message, __FUNCTION__, ##__VA_ARGS__))
#define STUPID_LOG_TRACEFN(message, ...) STUPID_DBG_SHOULD_LOG(stLogTrace(__FUNCTION__, STUPID_DBG_PARAM_FILE, STUPID_DBG_PARAM_LINE, message, ##__VA_ARGS__))
#else
#define STUPID_LOG_TRACE(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)
#define STUPID_LOG_TRACEFN(message, ...) _stLoggerStopUnusedArgumentWarning(message, ##__VA_ARGS__)

#endif // STUPID_LOG_TRACE_ENABLED
