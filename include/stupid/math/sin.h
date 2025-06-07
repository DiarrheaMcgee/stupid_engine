#pragma once

#include "stupid/common.h"
#include "stupid/assert.h"
#include "stupid/math/constants.h"
#include "stupid/math/lookup/lookup_sine.h"
#include "stupid/math/lookup/lookup_arccos.h"

/**
 * Sine approximation (LUT).
 * @note Precise to ~0.0001.
 * @return ~sin(x)
 */
static STUPID_INLINE f32 stSin(const f32 x)
{
	STUPID_STATIC_ASSERT(STUPID_IS_POWER2(STUPID_LOOKUP_SINEWAVE_SIZE), "sinewave lookup table size is not a power of 2");
	i32 index = ((f32)STUPID_LOOKUP_SINEWAVE_SIZE * x / (STUPID_MATH_TAU / 4.0));
	const u16 quad = (index >> 15) & 3;
	index &= STUPID_LOOKUP_SINEWAVE_SIZE - 1;
	switch (quad) {
	case 0:
		return STUPID_LOOKUP_SINEWAVE[index];
	case 1:
		return STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index];
	case 2:
		return -STUPID_LOOKUP_SINEWAVE[index];
	case 3:
		return -STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index];
	default:
		return 0.0;
	}
}

/**
 * Cosine approximation (LUT).
 * @note Precise to ~0.0001.
 * @return ~cos(x)
 */
static STUPID_INLINE f32 stCos(const f32 x)
{
	STUPID_STATIC_ASSERT(STUPID_IS_POWER2(STUPID_LOOKUP_SINEWAVE_SIZE), "sinewave lookup table size is not a power of 2");
	i32 index = ((f32)STUPID_LOOKUP_SINEWAVE_SIZE * x / (STUPID_MATH_TAU / 4.0));
	const u16 quad = (index >> 15) & 3;
	index &= STUPID_LOOKUP_SINEWAVE_SIZE - 1;
	switch (quad) {
	case 3:
		return STUPID_LOOKUP_SINEWAVE[index];
	case 0:
		return STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index];
	case 1:
		return -STUPID_LOOKUP_SINEWAVE[index];
	case 2:
		return -STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index];
	default:
		return 0.0;
	}
}

/**
 * Tangent approximation (LUT).
 * @note Precise to ~0.0001.
 * @return ~tan(x)
 */
static STUPID_INLINE f64 stTan(const f64 x)
{
	STUPID_STATIC_ASSERT(STUPID_IS_POWER2(STUPID_LOOKUP_SINEWAVE_SIZE), "sinewave lookup table size is not a power of 2");
	u16 index = (u16)((f32)STUPID_LOOKUP_SINEWAVE_SIZE * x / (STUPID_MATH_TAU / 4.0));
	const u16 quad = (index >> 15) & 3;
	index &= (STUPID_LOOKUP_SINEWAVE_SIZE - 1);
	switch (quad) {
	case 0:
		return STUPID_LOOKUP_SINEWAVE[index] / STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index];
	case 1:
		return -STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index] / STUPID_LOOKUP_SINEWAVE[index];
	case 2:
		return STUPID_LOOKUP_SINEWAVE[index] / STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index];
	case 3:
		return STUPID_LOOKUP_SINEWAVE[(STUPID_LOOKUP_SINEWAVE_SIZE - 1) - index] / STUPID_LOOKUP_SINEWAVE[index];
	default:
		return 0.0;
	}
}

/**
 * Inverse cosine approximation (LUT).
 * @note Precise to ~0.0001.
 * @return ~acos(x)
 */
static STUPID_INLINE f32 stAcos(const f32 x)
{
	STUPID_STATIC_ASSERT(STUPID_IS_POWER2(STUPID_LOOKUP_ARCCOS_SIZE), "arccos lookup table size is not a power of 2");
	if (STUPID_UNLIKELY(x >= 1.0))
		return 0.0;
	else if (STUPID_UNLIKELY(x <= -1.0))
		return STUPID_LOOKUP_ARCCOS[STUPID_LOOKUP_ARCCOS_SIZE - 1];

	u16 index = (u16)((f32)STUPID_LOOKUP_ARCCOS_SIZE * x) & (STUPID_LOOKUP_ARCCOS_SIZE - 1);
	return STUPID_LOOKUP_ARCCOS[index];
}

/**
 * Inverse sinewave approximation (LUT).
 * @note Precise to ~0.0001.
 * @return ~asin(x)
 */
#define stAsin(x) (-stAcos(x) + STUPID_MATH_TAUd2)

/**
 * Inverse tangent approximation.
 * @note Precise to ~0.00005.
 * return ~atan(x)
 */
static STUPID_INLINE f32 stAtan(const f32 x)
{
	const f32 a[3] = { 0.994766756708199f, -0.28543851807526100f, 0.0760699247645105f };
	const f32 xx = x * x;
	return ((a[2] * xx + a[1]) * xx + a[0]) * x;
}

/**
 * Inverse tangent approximation.
 * @note Precise to ~0.00005.
 * return ~atan2(x, y)
 */
static STUPID_INLINE f32 stAtan2(const f32 x, const f32 y)
{
	if (x > 0.0) return stAtan(y / x);
	else if (x < 0.0) return stAtan(y / -x);
	else return 0.0;
}

