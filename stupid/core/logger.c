#include "logger.h"

#include "common.h"
#include "core/thread.h"

#include <stdarg.h>
#include <stdio.h>

/// Used to make sure only one log is printed at a time.
static StMutex logger_lock = {0};

/// Buffer used for storing log messages.
/// @note This is statically allocated to avoid a stupid logger init function.
static char log_buffer[sizeof(StKb) * 8] = {0};

/// Used to prevent infinite recursion when logging an error with the logger.
static bool failed_to_print = false;

/// Used to replace replacable spam logs.
static bool last_log_is_replaceable = false;

/// ANSI text colors.
static const char colors[ST_TEXT_COLOR_MAX] = {
	'7', // white
	'1', // red
	'2', // green
	'3', // yellow
	'4', // blue
	'5', // purple
	'6', // cyan
	'0', // black
};

static const char *background_colors[ST_TEXT_BACKGROUND_MAX] = {
	"\0",  // nothing
	";47", // white
	";41", // red
	";42", // green
	";43", // yellow
	";44", // blue
	";45", // purple
	";46", // cyan
};

static const char *bright_background_colors[ST_TEXT_BACKGROUND_MAX] = {
	"\0", // nothing
	";107", // white
	";101", // red
	";102", // green
	";103", // yellow
	";104", // blue
	";105", // purple
	";106", // cyan
};

int stLogCustom(const st_text_color color, const st_text_background_color background_color, const st_text_property properties, const char *message, ...)
{
	if ((u32)color >= (u32)ST_TEXT_COLOR_MAX) {
		fprintf(stderr, "text color %u out of range\n", (u32)color);
		return -2;
	}

	else if ((u32)background_color >= (u32)ST_TEXT_BACKGROUND_MAX) {
		fprintf(stderr, "text background color %u out of range\n", (u32)background_color);
		return -3;
	}

	else if ((u32)properties >= (usize)ST_TEXT_PROPERTY_MAX) {
		fprintf(stderr, "text property %u out of range\n", (u32)properties);
		return -4;
	}

	if (!message) {
		STUPID_LOG_ERROR("stLogCustom() called with NULL message");
		return -1;
	}

	// output stream
	FILE *f;

	// use stderr if specified
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_ERR) f = stderr;
	else f = stdout;

	if (last_log_is_replaceable) fputs("\033[2K\r", f);

	// log that buf is no longer NULL if loggerInit() was called later than it should be
	if (failed_to_print) {
		failed_to_print = false;
		fputs("\033[94;1;4mlog buffer is no longer NULL\033[0m\n", stderr);
	}

	// lock the mutex to prevent multiple logs being printed at once
	stMutexLock(&logger_lock);

	// variadic arguments
	va_list pArg;
	va_start(pArg, message);
	vsnprintf(log_buffer, 1024, message, pArg);
	va_end(pArg);

	// use fputs if the text is plain
	if ((u32)color == 0 && (u32)background_color == 0 && (u32)properties == 0) {
		stMutexUnlock(&logger_lock);
		return fputs(log_buffer, f);
	}

	// extra properties for the text (like italic or bold)
	char property_str[9] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
	
	// incrementable pointer to property_str
	char *property_str_ptr = property_str;

	// enable bold text if requested
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_BOLD) {
		*property_str_ptr++ = ';';
		*property_str_ptr++ = '1';
	}

	// enable dim text if requested
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_DIM) {
		*property_str_ptr++ = ';';
		*property_str_ptr++ = '2';
	}

	// enable italic text if requested
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_ITALIC) {
		*property_str_ptr++ = ';';
		*property_str_ptr++ = '3';
	}

	// enable underlined text if requested
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_UNDERLINE) {
		*property_str_ptr++ = ';';
		*property_str_ptr++ = '4';
	}

	int b = 0;

	// print bright text if requested
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_BRIGHT) {
		// print bright text with a bright background if requested
		if ((u32)properties & (u32)ST_TEXT_PROPERTY_BRIGHT_BACKGROUND)
			b = fprintf(f, "\033[9%c%s%sm%s\033[0m", colors[(u32)color], property_str, bright_background_colors[(u32)background_color], log_buffer);

		else
			b = fprintf(f, "\033[9%c%s%sm%s\033[0m", colors[(u32)color], property_str, background_colors[(u32)background_color], log_buffer);
	}

	// otherwise print with normal brightness
	else {
		// print normal text with a bright background if requested
		if ((u32)properties & (u32)ST_TEXT_PROPERTY_BRIGHT_BACKGROUND)
			b = fprintf(f, "\033[3%c%s%sm%s\033[0m", colors[(u32)color], property_str, bright_background_colors[(u32)background_color], log_buffer);

		else
			b = fprintf(f, "\033[3%c%s%sm%s\033[0m", colors[(u32)color], property_str, background_colors[(u32)background_color], log_buffer);
	}

	if ((u32)properties & (u32)ST_TEXT_PROPERTY_FLUSH) fflush(f);
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_LINEBREAK) fputc('\n', f);
	if ((u32)properties & (u32)ST_TEXT_PROPERTY_BACKSPACE) {
		last_log_is_replaceable = true;
		fputc('\r', f);
	}
	else last_log_is_replaceable = false;

	stMutexUnlock(&logger_lock);

	return b;
}

/// strings at the start of different log levels
static const char *log_level_strings[] = {
	"\033[35;1;4;107m[CRIT]:   ",
	"\033[93;1;41m[FATAL]:	\033[0;93;41;4m",
	"\033[91;1m[ERROR]:  \033[0;31m",
	"\033[33;1m[WARN]:   \033[0;33;3m",
	"\033[96;1m[SYSTEM]: \033[0;96;3m",
	"\033[92;1m[INFO]:   \033[0;32m",
	"\033[34;1m[DEBUG]:  \033[0;34m",
	"\033[2;1m[SPAM]:   \033[0;2m",
	"\033[2;1m[TRACE]:  \033[0;2;3m"
};

void stLog(const st_log_level level, const char *message, ...)
{
	if ((u32)level >= ST_LOG_LEVEL_MAX) {
		STUPID_LOG_ERROR("invalid log level %u", (u32)level);
		return;
	}

	if (!message) {
		STUPID_LOG_ERROR("printCustom() called with NULL message");
		return;
	}

	// variadic arguments
	stMutexLock(&logger_lock);
	va_list pArg;
	va_start(pArg, message);
	vsnprintf(log_buffer, 1024, message, pArg);
	va_end(pArg);

	// output stream
	FILE *f;

	if (level == ST_LOG_LEVEL_CRITICAL || level == ST_LOG_LEVEL_FATAL || level == ST_LOG_LEVEL_ERROR)
		f = stderr;
	else
		f = stdout;

	// replace the last message if its replacable
	if (last_log_is_replaceable) {
		fputs("\033[2K\r", f);
		last_log_is_replaceable = false;
	}

	fprintf(f, "%s%s\033[0m", log_level_strings[level], log_buffer);

	// insert a \r at the end if required
	if (level == ST_LOG_LEVEL_SPAM) {
		putchar('\r');
		last_log_is_replaceable = true;
		fflush(stdout);
	}
	else
		fputc('\n', f);

	stMutexUnlock(&logger_lock);
}

void stLogTrace(const char *fn, const char *file, const int line, const char *message, ...)
{
	if (!message) {
		STUPID_LOG_ERROR("stLogTrace() called with NULL message");
		return;
	}

	stMutexLock(&logger_lock);
	va_list pArg;
	va_start(pArg, message);
	vsnprintf(log_buffer, 1024, message, pArg);
	va_end(pArg);

	printf("\033[2;1m[TRACE]:  \033[0;2;3m%s(): %s %s:%d\033[0m\n", fn, log_buffer, file, line);

	stMutexUnlock(&logger_lock);
}

void waitForInput(void)
{
	getchar();
}
