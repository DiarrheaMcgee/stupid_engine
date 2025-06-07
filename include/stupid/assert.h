/// @file assert.h
/// @brief Assertion functions, and some debugging stuff.
/// @author nonexistant

#pragma once

#include "stupid/common.h"
#include "stupid/clock.h"
#include "stupid/logger.h"

/**
 * @brief Kills the program in a memory leaking, but immediate way.
 * Triggers SIGILL so the kernel will kill the process.
 * @note This is an inline function instead of a macro to make debugging slightly easier.
 * @note Try to avoid using this.
 */
static STUPID_INLINE STUPID_NORETURN void STUPID_STOP(void)
{
        __builtin_trap();
}

/**
 * Internal logging function for assertion failure.
 * @param expr Expression that failed.
 * @param message Description of the failure.
 * @param file File where the assertion failure happened.
 * @param line Line where the assertion failure happened.
 * @note This is used a lot, so its not inlined to reduce the binary size.
 */
void _stAssertLog(const char *expr, const char *message, const char *file, const int line);

/**
 * Forcibly stops the engine if an expression evaluates to false.
 * @param expr Expression to be tested.
 * @param message Description of the failure.
 */
#define STUPID_ASSERT(expr, message)\
        do {\
                if (STUPID_UNLIKELY(!(expr))) {\
                        _stAssertLog(STUPID_DBG_IF_ELSE(" (" #expr ")", "\0"), message, __FILE__, __LINE__);\
                        STUPID_STOP();\
                }\
        } while (0)

#define STUPID_STATIC_ASSERT(expr, message) _Static_assert(expr, message)

/**
 * @brief Asserts that a pointer is not NULL.
 * @param p Any pointer.
 * @note Use this on all pointer arguments at the start of functions, except
 * for when you can be absolutely sure you dont need it.
 */
#define STUPID_NC(p) STUPID_ASSERT((p) != NULL, "pointer is NULL quitting to avoid segfault")

