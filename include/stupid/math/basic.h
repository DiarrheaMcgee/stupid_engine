#ifndef MATH_BASIC_H
#define MATH_BASIC_H

#include "stupid/common.h"
#include "stupid/math/constants.h"

// clang-format off

/**
 * Checks if an f64 is signed.
 * @param x A number.
 * @return True if x is signed.
 */
static STUPID_INLINE bool stSignbit(const f64 x)
{
        const union {f64 f; u64 i;} u = {x};
        return u.i >> 63;
}

/**
 * Checks if an f32 point number is signed.
 * @param x A number.
 * @return True if x is signed.
 */
static STUPID_INLINE int stSignbitf(const f32 x)
{
        const union {f32 f; u32 i;} u = {x};
        return u.i >> 31;
}

/**
 * Checks if an integer is signed.
 * @param x A number.
 * @return True if x is signed.
 */
#define stSignbiti(x) _Generic((x),\
                      i64: (u64)x >> 63,\
                      i32: (u32)x >> 31,\
                      i16: (u16)x >> 15,\
                      i8:  (u8)x  >> 7,\
                      default: (u64)x >> (sizeof(x) * 8 - 1))

/**
 * Checks if a number is signed.
 * @param x A number.
 * @return True if x is signed.
 */
#define stSignbit(x) _Generic((x),\
                     f64:     stSignbit(x),\
                     f32:     stSignbitf(x),\
                     default: stSignbiti(x))

/**
 * @brief Makes a number positive.
 * Unfortunately, a simple bitwise & cant be done here because of those pesky xmm registers.
 * @param x A number.
 * @return |x|
 * @note From https://git.musl-libc.org/cgit/musl/tree/src/math/x86_64/fabs.c.
 */
static STUPID_INLINE f64 stFabs(f64 x)
{
	f64 t;
	__asm__ ("pcmpeqd %0, %0" : "=x"(t));          // t = ~0
	__asm__ ("psrlq   $1, %0" : "+x"(t));          // t >>= 1
	__asm__ ("andps   %1, %0" : "+x"(x) : "x"(t)); // x &= t
	return x;

}

/**
 * f32 abs implementation stolen from musl libc.
 * Unfortunately, a simple bitwise & cant be done here because of those pesky xmm registers.
 * @param x A number.
 * @return |x|
 * @note From https://git.musl-libc.org/cgit/musl/tree/src/math/x86_64/fabsf.c.
 */
static STUPID_INLINE f32 stFabsf(f32 x)
{
	f32 t;
	__asm__ ("pcmpeqd %0, %0" : "=x"(t));          // t = ~0
	__asm__ ("psrld   $1, %0" : "+x"(t));          // t >>= 1
	__asm__ ("andps   %1, %0" : "+x"(x) : "x"(t)); // x &= t
	return x;
}

/**
 * Integer abs implementation.
 * @param x an integer
 * @return |x|
 * @note Practically free.
 */
static STUPID_INLINE i64 stAbs(const i64 x)
{
        if (x < 0) return -x;
        else return x;
}

/**
 * Returns x as a positive number.
 * @param x A number.
 * @return |x|
 */
#define stAbs(x) _Generic((x),\
                 f64: stFabs,\
                 f32: stFabsf,\
                 i64: stAbs,\
                 i32: stAbs,\
                 i16: stAbs,\
                 i8:  stAbs)(x)

/**
 * Rounds an f64 to the nearest integer
 * @param x A f64.
 * @return [x]
 */
static STUPID_INLINE f64 stRound(f64 x)
{
        __asm__("frndint" : "+t"(x));
        return x;
}

/**
 * Rounds x to the nearest integer towards 0.
 * @param x A number.
 * @return x rounded down.
 * @note From https://git.musl-libc.org/cgit/musl/blame/src/math/floor.c.
 */
static STUPID_INLINE f64 stFloor(const f64 x)
{
	static const f64 toint = 1.0/STUPID_F64_EPSILON;

	union {f64 f; u64 i;} u = {x};
	int e = u.i >> 52 & 0x7ff;
	volatile f64 y;

	if (e >= 0x3ff+52 || x == 0)
		return x;
	/* y = int(x) - x, where int(x) is an integer neighbor of x */
	if (u.i >> 63)
		y = x - toint + toint - x;
	else
		y = x + toint - toint - x;
	/* special case because of non-nearest rounding modes */
	if (e <= 0x3ff-1) {
		y;
		return u.i >> 63 ? -1 : 0;
	}
	if (y > 0)
		return x + y - 1;
	return x + y;

}

/**
 * Truncates the decimal from an f64.
 * @param x A number.
 * @return x Without a decimal.
 */
static STUPID_INLINE f64 stTrunc(const f64 x)
{
        return (f64)((i64)(x));
}

/**
 * Rounds x to the nearest integer towards infinity.
 * @param x A number.
 * @return x rounded up.
 * @note From https://git.musl-libc.org/cgit/musl/blame/src/math/ceil.c.
 */
static STUPID_INLINE f64 stCeil(const f64 x)
{
        static const f64 toint = 1/0x1p-52;

	const union {f64 f; u64 i;} u = {x};
	const i32 e = u.i >> 52 & 0x7ff;
	volatile f64 y;

	if (e >= 0x3ff+52 || x == 0)
		return x;
	/* y = int(x) - x, where int(x) is an integer neighbor of x */
	if (u.i >> 63)
		y = x - toint + toint - x;
	else
		y = x + toint - toint - x;
	/* special case because of non-nearest rounding modes */
	if (e <= 0x3ff-1) {
		y;
		return u.i >> 63 ? -0.0 : 1;
	}
	if (y < 0)
		return x + y + 1;
	return x + y;
}

#endif
