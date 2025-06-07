#ifndef MATH_EXP_INCLUDED
#define MATH_EXP_INCLUDED

#include "stupid/common.h"

/**
 * @brief Single asm instruction sqrt implementation for f64s.
 * Finds the number that when squared produces x.
 * @param x A number.
 * @return The number that when squared produces x.
 */
static STUPID_INLINE f64 stSqrt(f64 x)
{
        __asm__("sqrtsd %0, %0" : "+x"(x));
        return x;
}

/**
 * @brief Single asm instruction sqrt implementation for f32s.
 * Finds the number that when squared produces x.
 * @param x A number.
 * @return The number that when squared produces x.
 */
static STUPID_INLINE f32 stSqrtf(f32 x)
{
        __asm__("sqrtss %0, %0" : "+x"(x));
        return x;
}

/**
 * @brief Single asm instruction inverse sqrt implementation for f32s.
 * Finds the inverse of the number that when squared produces x.
 * @param x A positive number.
 * @return 1 / sqrt(x)
 */
static STUPID_INLINE f32 stISqrt(f32 x)
{
        __asm__("rsqrtss %0, %0" : "+x"(x));
        return x;
}

#define stSqrt(x) _Generic((x),\
                  f64: stSqrt,\
                  f32: stSqrtf,\
                  i64: stSqrt,\
                  i32: stSqrtf,\
                  i16: stSqrtf,\
                  i8:  stSqrtf,\
                  u64: stSqrt,\
                  u32: stSqrtf,\
                  u16: stSqrtf,\
                  u8:  stSqrtf)(x)

/**
 * @brief A bland assembly language log2 implementation.
 * Finds the number you need to raise 2 by in order to get x.
 * @param x A non zero number.
 * @return the power 2 must be raised by in order to get x
 */
static STUPID_INLINE f32 stLog2(f32 x)
{
        __asm__("fld1");
        __asm__("fyl2x" : "+t"(x));
        return x;
}

/**
 * @brief A ln(x) implementation.
 * Finds the power e is raised by in order to get x.
 * @param x A positive non zero number.
 * @return The power e must be raised by in order to get x.
 * @note From https://quadst.rip/ln-approx.html.
 */
static inline f32 stLn(f32 x)
{
	// ASSUMING: 
	// - non-denormalized numbers i.e. x > 2^-126
	// - integer is 32 bit. float is IEEE 32 bit.

	// INSPIRED BY:
	// - https://stackoverflow.com/a/44232045
	// - http://mathonweb.com/help_ebook/html/algorithms.htm#ln
	// - https://en.wikipedia.org/wiki/Fast_inverse_square_root

	// FORMULA: 
	// x = m * 2^p =>
	//   ln(x) = ln(m) + ln(2)p,

	// first normalize the value to between 1.0 and 2.0
	// assuming normalized IEEE float
	//    sign  exp       frac
	// 0b 0    [00000000] 00000000000000000000000
	// value = (-1)^s * M * 2 ^ (exp-127)
	//
	// exp = 127 for x = 1, 
	// so 2^(exp-127) is the multiplier

	// evil floating point bit level hacking
	void *p = &x;
	u32 bx = *((u32 *)p);

	// extract exp, since x>0, sign bit must be 0
	u32 ex = bx >> 23;
	i32 t = (i32)ex-(i32)127;
	//u32 s = (t < 0) ? (-t) : t;

	// reinterpret back to float
	//   127 << 23 = 1065353216
	//   0b11111111111111111111111 = 8388607
	bx = 1065353216 | (bx & 8388607);
	p = &bx;
	x = *((f32 *)p);


	// use remez algorithm to find approximation between [1,2]
	// - see this answer https://stackoverflow.com/a/44232045
	// - or this usage of C++/boost's remez implementation
	//   https://computingandrecording.wordpress.com/2017/04/24/
	// e.g.
	// boost::math::tools::remez_minimax<double> approx(
	//    [](const double& x) { return log(x); },
	// 4, 0, 1, 2, false, 0, 0, 64);
	//
	// 4th order is:
	// { -1.74178, 2.82117, -1.46994, 0.447178, -0.0565717 }
	// 
	// 3rd order is:
	// { -1.49278, 2.11263, -0.729104, 0.10969 }

	return 

		/* less accurate */
		//-1.49278+(2.11263+(-0.729104+0.10969*x)*x)*x      

		/* OR more accurate */      
		-1.7417939+(2.8212026+(-1.4699568+(0.44717955-0.056570851*x)*x)*x)*x

		/* compensate for the ln(2)s. ln(2)=0.6931471806 */    
		+ 0.6931471806*t;
}

#endif
