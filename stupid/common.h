/// @file common.h
/// @brief Stuff available in all files in the engine.
/// This is mostly for types and useful macros.
/// @author nonexistant

#pragma once

//#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

/// Indicates that a function never returns.
#define STUPID_NORETURN  _Noreturn

/// Makes a variable compatible with atomic operations.
/// @note This does not make the variable thread safe unless you use atomic operations.
#define STUPID_ATOMIC _Atomic

#ifdef _DEBUG

/// Evaluates a function block only in debug mode.
#define STUPID_DBG(x) x

/// Evaluates a function block only in release mode.
#define STUPID_NDBG(x)

/// Evaluates x in debug mode, and y in release mode.
#define STUPID_DBG_IF_ELSE(x, y) x

/// Debug parameter for the filename.
#define STUPID_DBG_PARAM_FILE       ___dbg_param_filename

/// Debug parameter for the line number.
#define STUPID_DBG_PARAM_LINE       ___dbg_param_line

/// Debug parameter indicating weather the function should print logs or not.
#define STUPID_DBG_PARAM_SHOULD_LOG ___dbg_param_should_log

/// Parameters passed to most functions, just to avoid needing to type this for each function call.
/// Its really annoying to use, and it means that most functions need to be called by macros, but the upside is
/// that functions can print the file and line they are called from without too much trouble.
#define STUPID_DBG_PARAMS , ((const char *)__FILE__), __LINE__, true

/// See STUPID_DBG_PARAMS.
/// @note Doesnt print logs.
#define STUPID_DBG_PARAMS_NL , NULL, 0, false

/// @brief See STUPID_DBG_PARAMS.
/// Used for function prototypes.
#define STUPID_DBG_PROTO_PARAMS , const char *STUPID_DBG_PARAM_FILE, const int STUPID_DBG_PARAM_LINE, const bool STUPID_DBG_PARAM_SHOULD_LOG

#else

/// Evaluates a function block only in debug mode.
#define STUPID_DBG(x)

/// Evaluates a function block only in release mode.
#define STUPID_NDBG(x) x

/// Evaluates x in debug mode, and y in release mode.
#define STUPID_DBG_IF_ELSE(x, y) y

/// Debug parameter for the filename.
#define STUPID_DBG_PARAM_FILE   "NULL"

/// Debug parameter for the line number.
#define STUPID_DBG_PARAM_LINE       0

/// Debug parameter indicating weather the function should print logs or not.
#define STUPID_DBG_PARAM_SHOULD_LOG false

/// Parameters passed to most functions, just to avoid needing to type this for each function call.
/// Its a bit annoying to use, and it means that most functions need to be called by macros, but it means
/// that functions can print the file and line they are called from without too much trouble.
#define STUPID_DBG_PARAMS

/// See STUPID_DBG_PARAMS.
/// @note Doesnt print logs.
#define STUPID_DBG_PARAMS_NL

/// @brief See STUPID_DBG_PARAMS.
/// Used for function prototypes.
#define STUPID_DBG_PROTO_PARAMS

#endif

#if defined(__GNUC__) && (__GNUC__ >= 3)

/// Aligns a struct to the size of a type.
#define STUPID_ALIGN(alignment) __attribute__ ((__aligned__(alignment)))

/// Enables aliasing for a type or variable.
#define STUPID_MAY_ALIAS __attribute__ ((__may_alias__))

/// Disables warnings for a variable being unused.
#define STUPID_UNUSED __attribute__ ((unused))

/// Forces a function to be always inline.
#define STUPID_INLINE inline __attribute__ ((__always_inline__))

/// Forces a function to never be inline.
#define STUPID_NOINLINE __attribute__ ((__noinline__))

/// Indicates that a function allocates or reallocates memory (for optimization purposes).
#define STUPID_ATTR_MALLOC __attribute__ ((malloc))

/// Indicates that a function takes printf format arguments (mainly useful for compiler warnings).
#define STUPID_ATTR_FORMAT(fmt, index) __attribute__ ((format(printf, fmt, index)))

/// Tells the compiler that a condition is likely (for optimization purposes).
#define STUPID_LIKELY(x) __builtin_expect(!!(x), 1)

/// Tells the compiler that a condition is unlikely (for optimization purposes).
#define STUPID_UNLIKELY(x) __builtin_expect(!!(x), 0)

/// Indicates that something is going to be replaced or removed soon, and should not be used.
#define STUPID_DEPRECATED(str) __attribute__ ((deprecated(str)))

#else

#define STUPID_ALIGN(alignment)

/// Enables aliasing for a type or variable.
#define STUPID_MAY_ALIAS

/// Disables warnings for a variable being unused.
#define STUPID_UNUSED

/// Forces a function to be always inline.
#define STUPID_INLINE

/// Forces a function to never be inline.
#define STUPID_NOINLINE

/// Indicates that a function allocates or reallocates memory, for optimization purposes.
#define STUPID_ATTR_MALLOC

/// Indicates that a function takes printf format arguments (mainly useful for warnings).
#define STUPID_ATTR_FORMAT(fmt, index)

/// Tells the compiler that a condition is unlikely, for optimization purposes.
#define STUPID_UNLIKELY(x) (x)

/// Tells the compiler that a condition is likely, for optimization purposes.
#define STUPID_LIKELY(x) (x)

/// Indicates that something is going to be replaced or removed soon, and should not be used.
#define STUPID_DEPRECATED(str)

#endif

/// Evaluates x if in debug mode and STUPID_DBG_PARAM_SHOULD_LOG is true.
#define STUPID_DBG_SHOULD_LOG(x) STUPID_DBG(do {if (STUPID_DBG_PARAM_SHOULD_LOG) { x; }} while (0))

/// unsigned 64 bit integer
typedef uint64_t u64;

/// unsigned 32 bit integer
typedef uint32_t u32;

/// unsigned 16 bit integer
typedef uint16_t u16;

/// unsigned 8 bit integer
typedef uint8_t  u8;

/// signed 64 bit integer
typedef int64_t  i64;

/// signed 32 bit integer
typedef int32_t  i32;

/// signed 16 bit integer
typedef int16_t  i16;

/// signed 8 bit integer
typedef int8_t   i8;

/// 64 bit double
typedef double   f64;

/// 32 bit float
typedef float    f32;

/// architecture dependant unsigned integer used for indexing
typedef size_t   usize;

/// One Kilobyte.
/// @note Equivalent to 1024 bytes.
typedef u8 StKb[1024];

/// One Megabyte.
/// @note Equivalent to 1024 Kilobytes.
typedef u8 StMb[1024 * 1024];

/// One Gigabyte.
/// @note Equivalent to 1024 Megabytes.
typedef u8 StGb[1024 * 1024 * 1024];

/// Aligns a struct to 1 byte.
#define STUPID_ALIGN1  STUPID_ALIGN(1)

/// Aligns a struct to 2 bytes.
#define STUPID_ALIGN2  STUPID_ALIGN(2)

/// Aligns a struct to 4 bytes.
#define STUPID_ALIGN4  STUPID_ALIGN(4)

/// Aligns a struct to 8 bytes.
#define STUPID_ALIGN8  STUPID_ALIGN(8)

/// Aligns a struct to 16 bytes.
#define STUPID_ALIGN16 STUPID_ALIGN(16)

/// Aligns a struct to 32 bytes.
#define STUPID_ALIGN32 STUPID_ALIGN(32)

/// Aligns a struct to 1 byte and enables aliasing.
#define STUPID_A1 STUPID_ALIGN1  STUPID_MAY_ALIAS

/// Aligns a struct to 2 bytes and enables aliasing.
#define STUPID_A2 STUPID_ALIGN2  STUPID_MAY_ALIAS

/// Aligns a struct to 4 bytes and enables aliasing.
#define STUPID_A4 STUPID_ALIGN4  STUPID_MAY_ALIAS

/// Aligns a struct to 8 bytes and enables aliasing.
#define STUPID_A8 STUPID_ALIGN8  STUPID_MAY_ALIAS

/// Aligns a struct to 16 bytes and enables aliasing.
#define STUPID_STUPID_A16 STUPID_ALIGN16 STUPID_MAY_ALIAS

/// Aligns a struct to 32 bytes and enables aliasing.
#define STUPID_A32 STUPID_ALIGN32 STUPID_MAY_ALIAS

/**
 * Clamps a value between min and max.
 * @param A number.
 * @param min Minimum value for the input variable.
 * @param max Maximum value for the input variable.
 */
#define STUPID_CLAMP(value, min, max) (((value) <= min) ? min : ((value) >= max) ? max : value)

/**
 * Picks the smaller value.
 * @param x A number.
 * @param y A number.
 */
#define STUPID_MIN(x, y) (((x) < (y)) ? (x) : (y))

/**
 * Picks the larger value.
 * @param x A number.
 * @param y A number.
 */
#define STUPID_MAX(x, y) (((x) > (y)) ? (x) : (y))

/**
 * Swaps 2 variables of the same type.
 * @param x A number.
 * @param y A number.
 */
#define STUPID_SWAP(x, y) do {__typeof__(x) tmp = x; x = y; y = tmp;} while (0)

/**
 * Returns true if a number is NaN, otherwise false.
 * @param x A number.
 */
#define STUPID_IS_NAN(x) _Generic((x),\
                  f64:     ((x) != (x)),\
                  f32:     ((x) != (x)),\
                  default: false)

/// Checks if a number is a power of 2.
#define STUPID_IS_POWER2(x) ((x) != 0 && ((x) & ((x) - 1)) == 0)

