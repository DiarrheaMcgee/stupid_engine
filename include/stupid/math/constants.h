#pragma once

#include "stupid/common.h"

/// Infinity (at least as far as floating point numbers are concerned).
#define STUPID_INF 1e30

/// Smallest possible f32 value.
#define STUPID_F32_EPSILON 1.192092896e-07

/// Smallest possible f64 value.
#define STUPID_F64_EPSILON 0x1p-52

/// @brief Number of radians in a circle.
/// I would have called this "C" but i dont get to decide.
/// @note Very useful.
#define STUPID_MATH_TAU 6.28318630717908647711

/// Tau / 2.
/// @note Sorta useless.
#define STUPID_MATH_TAUd2 3.14159265358979323846

/// Tau / 4.
/// @note Sorta useful.
#define STUPID_MATH_TAUd4 1.57079632679489661923

/// Tau / 8.
/// @note Sorta useful.
#define STUPID_MATH_TAUd8 0.78539816339744830961

/// Half the radians in a circle.
/// @note Deprecated, and useless.
static const f64 STUPID_DEPRECATED("Use tau instead.") STUPID_MATH_PI = STUPID_MATH_TAUd2;

/// sqrt(2)
#define STUPID_SQRT2 1.41421356237309504880

/// sqrt(3)
#define STUPID_SQRT3 1.73205080756887729352

/// The base of ln and, the mathematical definition of exponential growth.
/// @note This is the best number.
#define STUPID_MATH_E 2.71828182845904523536

/// e ** 2
#define STUPID_MATH_E2 7.38905609893065022722

/// e ** 3
#define STUPID_MATH_E3 20.0855369231876677408

/// e ** 4
#define STUPID_MATH_E4 54.5981500331442390779

/// e ** 5
#define STUPID_MATH_E5 148.413159102576603420

/// e ** 6
#define STUPID_MATH_E6 403.428793492735122607

/// ln(2)
#define STUPID_MATH_LN2 0.69314718055994530941

/// ln(10)
#define STUPID_MATH_LN10 2.30258509299404568401
