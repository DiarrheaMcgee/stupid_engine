// ABANDON ALL HOPE YE WHO ENTER HERE

#pragma once

#include "stupid/common.h"
#include "stupid/math/basic.h"
#include "stupid/math/sin.h"
#include "stupid/math/exp.h"

#include <immintrin.h>

typedef union StVec4 {
	f32 v[4];

	struct {
		f32 x, y, z, w;
	};
} StVec4;

typedef union StVec3 {
	f32 v[3];

	struct {
		f32 x, y, z;
	};
} StVec3;

typedef union StVec2 {
	f32 v[2];

	struct {
		f32 x, y;
	};
} StVec2;

typedef struct StMat4 {
	f32 m[4][4];
} StMat4;

typedef struct StMat3 {
	f32 m[3][3];
} StMat3;

typedef struct StMat2 {
	f32 m[2][2];
} StMat2;

typedef union StQuat {
	struct {
		f32 w;

		union {
			StVec3 v;

			struct {
				f32 x, y, z;
			};
		};
	};

	f32 q[4];
} StQuat;

#define STVEC2(x, y) ((StVec2){{(x), (y)}})
#define STVEC3(x, y, z) ((StVec3){{(x), (y), (z)}})
#define STVEC4(x, y, z, w) ((StVec4){{(x), (y), (z), (w)}})

/// 4D f32 vector.
//typedef f32 StVec4[4];
//
///// 3D f32 vector.
///// @note Contains 4 elements instead of 3 to make more vectorization possible.
//typedef f32 StVec3[4];
//
///// 2D f32 vector.
//typedef f32 StVec2[2];
//
///// 4x4 f32 matrix.
//typedef f32 StMat4[4][4];
//
///// 3x3 f32 matrix.
//typedef f32 StMat3[3][3];
//
///// 2x2 f32 matrix.
//typedef f32 StMat2[2][2];

/**
 * Returns a zerofilled StVec4.
 * @return {0.0, 0.0, 0.0, 0.0}
 */
static STUPID_INLINE StVec4 stVec4Zero(void)
{
	StVec4 vec;
	*(__m128*)&vec = _mm_setzero_ps();
	return vec;
}

/**
 * Returns a zerofilled StVec3.
 * @return {0, 0, 0}
 */
static STUPID_INLINE StVec3 stVec3Zero(void)
{
	StVec3 vec = {0};
	return vec;
}

/**
 * Returns a zerofilled StVec2.
 * @return {0, 0}
 */
static STUPID_INLINE StVec2 stVec2Zero(void)
{
	StVec2 vec = {0};
	*(__m128*)&vec = _mm_setzero_ps();
	return vec;
}

/**
 * Returns a new StVec4 with all elements set to the specified value.
 * @param value Value to set all elements of the new vector to.
 * @return {value, value, value, value}
 */
static STUPID_INLINE StVec4 stVec4Set(const f32 value)
{
	StVec4 vec;
	*(__m128*)&vec = _mm_set1_ps(value);
	return vec;
}

/**
 * Returns a new StVec3 with all elements set to the specified value.
 * @param value Value to set all elements of the new vector to.
 * @return {value, value, value}
 */
static STUPID_INLINE StVec3 stVec3Set(const f32 value)
{
	StVec3 vec = {.x = value, .y = value, .z = value};
	return vec;
}

/**
 * Returns a new StVec2 with all elements set to the specified value.
 * @param value Value to set all elements of the new vector to.
 * @return {value, value}
 */
static STUPID_INLINE StVec2 stVec2Set(const f32 value)
{
	StVec2 vec;
	vec.v[0] = value;
	vec.v[1] = value;
	return vec;
}

/**
 * Adds two StVec4s.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] + y[0], x[1] + y[1], x[2] + y[2], x[3] + y[3]}
 */
static STUPID_INLINE StVec4 stVec4Add(const StVec4 x, const StVec4 y)
{
	StVec4 vec;
	*(__m128*)&vec = _mm_add_ps(*(__m128*)&x, *(__m128*)&y);
	return vec;
}

/**
 * Adds two StVec3s.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] + y[0], x[1] + y[1], x[2] + y[2]}
 */
static STUPID_INLINE StVec3 stVec3Add(const StVec3 x, const StVec3 y)
{
	StVec3 vec;
	vec.x = x.x + y.x;
	vec.y = x.y + y.y;
	vec.z = x.z + y.z;
	return vec;
}

/**
 * Adds two StVec2s.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] + y[0], x[1] + y[1]}
 */
static STUPID_INLINE StVec2 stVec2Add(const StVec2 x, const StVec2 y)
{
	StVec2 vec;
	vec.v[0] = x.v[0] + y.v[0];
	vec.v[1] = x.v[1] + y.v[1];
	return vec;
}

/**
 * Subtracts one StVec4 from another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] - y[0], x[1] - y[1], x[2] - y[2], x[3] - y[3]}
 */
static STUPID_INLINE StVec4 stVec4Sub(const StVec4 x, const StVec4 y)
{
	StVec4 vec;
	*(__m128*)&vec = _mm_sub_ps(*(__m128*)&x, *(__m128*)&y);
	return vec;
}

/**
 * Subtracts one StVec3 from another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] - y[0], x[1] - y[1], x[2] - y[2]}
 */
static STUPID_INLINE StVec3 stVec3Sub(const StVec3 x, const StVec3 y)
{
	StVec3 vec;
	vec.x = x.x - y.x;
	vec.y = x.y - y.y;
	vec.z = x.z - y.z;
	return vec;
}

/**
 * Subtracts one StVec2 from another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] - y[0], x[1] - y[1]}
 */
static STUPID_INLINE StVec2 stVec2Sub(const StVec2 x, const StVec2 y)
{
	StVec2 vec;
	vec.v[0] = x.v[0] - y.v[0];
	vec.v[1] = x.v[1] - y.v[1];
	return vec;
}

/**
 * Multiplies one StVec4 by another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] * y[0], x[1] * y[1], x[2] * y[2], x[3] * y[3]}
 */
static STUPID_INLINE StVec4 stVec4Mul(const StVec4 x, const StVec4 y)
{
	StVec4 vec;
	*(__m128*)&vec = _mm_mul_ps(*(__m128*)&x, *(__m128*)&y);
	return vec;
}

/**
 * Multiplies one StVec3 by another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] * y[0], x[1] * y[1], x[2] * y[2]}
 */
static STUPID_INLINE StVec3 stVec3Mul(const StVec3 x, const StVec3 y)
{
	StVec3 vec;
	vec.x = x.x * y.x;
	vec.y = x.y * y.y;
	vec.z = x.z * y.z;
	return vec;
}

/**
 * Multiplies one StVec2 by another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] * y[0], x[1] * y[1]}
 */
static STUPID_INLINE StVec2 stVec2Mul(const StVec2 x, const StVec2 y)
{
	StVec2 vec;
	vec.v[0] = x.v[0] * y.v[0];
	vec.v[1] = x.v[1] * y.v[1];
	return vec;
}

/**
 * Divides one StVec4 by another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] / y[0], x[1] / y[1], x[2] / y[2], x[3] / y[3]}
 */
static STUPID_INLINE StVec4 stVec4Div(const StVec4 x, const StVec4 y)
{
	StVec4 vec;
	*(__m128*)&vec = _mm_div_ps(*(__m128*)&x, *(__m128*)&y);
	return vec;
}

/**
 * Divides one StVec3 by another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] / y[0], x[1] / y[1], x[2] / y[2]}
 */
static STUPID_INLINE StVec3 stVec3Div(const StVec3 x, const StVec3 y)
{
	StVec3 vec;
	vec.x = x.x / y.x;
	vec.y = x.y / y.y;
	vec.z = x.z / y.z;
	return vec;
}

/**
 * Divides one StVec2 by another.
 * @param x First vector.
 * @param y Second vector.
 * @return {x[0] / y[0], x[1] / y[1]}
 */
static STUPID_INLINE StVec2 stVec2Div(const StVec3 x, const StVec3 y)
{
	StVec2 vec;
	vec.v[0] = x.v[0] / y.v[0];
	vec.v[1] = x.v[1] / y.v[1];
	return vec;
}

/**
 * Scales all elements in a StVec4 by the specified value.
 * @param vec Vector to scale.
 * @param scale Number to scale all elements in the specified vector by.
 * @return {vec[0] * scale, vec[1] * scale, vec[2] * scale, vec[3] * scale}
 */
static STUPID_INLINE StVec4 stVec4Scale(const StVec4 vec, const f32 scale)
{
	StVec4 result;
	*(__m128*)&result = _mm_mul_ps(*(__m128*)&vec, _mm_set1_ps(scale));
	return result;
}

/**
 * Scales all elements in a StVec3 by the specified value.
 * @param vec Vector to scale.
 * @param scale Number to scale all elements in the specified vector by.
 * @return {vec[0] * scale, vec[1] * scale, vec[2] * scale}
 */
static STUPID_INLINE StVec3 stVec3Scale(const StVec3 vec, const f32 scale)
{
	StVec3 result;
	result.x = vec.x * scale;
	result.y = vec.y * scale;
	result.z = vec.z * scale;
	return result;
}

/**
 * Scales all elements in a StVec2 by the specified value.
 * @param vec Vector to scale.
 * @param scale Number to scale all elements in the specified vector by.
 * @return {vec[0] * scale, vec[1] * scale}
 */
static STUPID_INLINE StVec2 stVec2Scale(const StVec2 vec, const f32 scale)
{
	StVec2 result;
	result.v[0] = vec.v[0] * scale;
	result.v[1] = vec.v[1] * scale;
	return result;
}

/**
 * Divides all elements in a StVec4 by the specified value.
 * @param vec Vector to scale.
 * @param scale Number to divide all elements in the specified vector by.
 * @return {vec[0] / scale, vec[1] / scale, vec[2] / scale, vec[3] / scale}
 */
static STUPID_INLINE StVec4 stVec4ScaleInv(const StVec4 vec, const f32 scale)
{
	StVec4 result;
	*(__m128*)&result = _mm_div_ps(*(__m128*)&vec, _mm_set1_ps(scale));
	return result;
}

/**
 * Divides all elements in a StVec3 by the specified value.
 * @param vec Vector to scale.
 * @param scale Number to divide all elements in the specified vector by.
 * @return {vec[0] / scale, vec[1] / scale, vec[2] / scale}
 */
static STUPID_INLINE StVec3 stVec3ScaleInv(const StVec3 vec, const f32 scale)
{
	StVec3 result;
	result.x = vec.x / scale;
	result.y = vec.y / scale;
	result.z = vec.z / scale;
	return result;
}

/**
 * Divides all elements in a StVec2 by the specified value.
 * @param vec Vector to scale.
 * @param scale Number to divide all elements in the specified vector by.
 * @return {vec[0] / scale, vec[1] / scale}
 */
static STUPID_INLINE StVec2 stVec2ScaleInv(const StVec2 vec, const f32 scale)
{
	StVec2 result;
	result.v[0] = vec.v[0] / scale;
	result.v[1] = vec.v[1] / scale;
	return result;
}

/**
 * Gets the sum of a StVec4.
 * @param vec Vector to get the sum of.
 * @return vec[0] + vec[1] + vec[2] + vec[3]
 */
static STUPID_INLINE f32 stVec4Sum(StVec4 vec)
{
	__m128 tmp = _mm_add_ps(*(__m128*)&vec, *(__m128*)&vec);
	__m128 res = _mm_add_ss(tmp, _mm_shuffle_ps(tmp, tmp, 1));
	return _mm_extract_ps(res, 0);
}

/**
 * Gets the sum of a StVec3.
 * @param vec Vector to get the sum of.
 * @return vec[0] + vec[1] + vec[2]
 */
static STUPID_INLINE f32 stVec3Sum(StVec3 vec)
{
	// im not sure how to properly optimize this so il let the compiler figure it out
	return vec.v[0] + vec.v[1] + vec.v[2];
}

/**
 * Gets the sum of a StVec2.
 * @param vec Vector to get the sum of.
 * @return vec[0] + vec[1]
 */
static STUPID_INLINE f32 stVec2Sum(StVec2 vec)
{
	return vec.v[0] + vec.v[1];
}

/**
 * Gets the cross product ov 2 StVec3s.
 * @param x First vector.
 * @param y Second vector.
 * @return
 */
static STUPID_INLINE StVec3 stVec3Cross(const StVec3 x, const StVec3 y)
{
	StVec3 result;
	result.v[0] = x.v[1] * y.v[2] - x.v[2] * y.v[1];
	result.v[1] = x.v[2] * y.v[0] - x.v[0] * y.v[2];
	result.v[2] = x.v[0] * y.v[1] - x.v[1] * y.v[0];
	return result;
}

/**
 * Gets the dot product of 2 StVec4s.
 * @param x First vector.
 * @param y Second vector.
 * @return sum({x[0] * y[0], x[1] * y[1], x[2] * y[2], x[3] * y[3]})
 */
static STUPID_INLINE f32 stVec4Dot(const StVec4 x, const StVec4 y)
{
	return stVec4Sum(stVec4Mul(x, y));
}

/**
 * Gets the dot product of 2 StVec3s.
 * @param x First vector.
 * @param y Second vector.
 * @return sum({x[0] * y[0], x[1] * y[1], x[2] * y[2]})
 */
static STUPID_INLINE f32 stVec3Dot(const StVec3 x, const StVec3 y)
{
	return stVec3Sum(stVec3Mul(x, y));
}

/**
 * Gets the dot product of 2 StVec2s.
 * @param x First vector.
 * @param y Second vector.
 * @return sum({x[0] * y[0], x[1] * y[1])
 */
static STUPID_INLINE f32 stVec2Dot(const StVec2 x, const StVec2 y)
{
	return stVec2Sum(stVec2Mul(x, y));
}

/**
 * Gets the hypotenuse of a StVec4.
 * @param vec Vector to get the hypotenuse of.
 * @return sqrt(sum(x * x))
 */
static STUPID_INLINE f32 stVec4Hypot(const StVec4 vec)
{
	return stSqrtf(stVec4Sum(stVec4Mul(vec, vec)));
}

/**
 * Gets the hypotenuse of a StVec3.
 * @param vec Vector to get the hypotenuse of.
 * @return sqrt(sum(x * x))
 */
static STUPID_INLINE f32 stVec3Hypot(const StVec3 vec)
{
	return stSqrtf(stVec3Sum(stVec3Mul(vec, vec)));
}

/**
 * Gets the hypotenuse of a StVec2.
 * @param vec Vector to get the hypotenuse of.
 * @return sqrt(sum(x * x))
 */
static STUPID_INLINE f32 stVec2Hypot(const StVec2 vec)
{
	return stSqrtf(stVec2Sum(stVec2Mul(vec, vec)));
}

/**
 * Calculates the distance between 2 StVec4s.
 * @param x First vector.
 * @param y Second vector.
 * @return sqrt(sum((y - x) ^ 2))
 */
static STUPID_INLINE f32 stVec4Distance(const StVec4 x, const StVec4 y)
{
	return stSqrtf(stVec4Sum(stVec4Mul(stVec4Sub(y, x), stVec4Sub(y, x))));
}

/**
 * Calculates the distance between 2 StVec3s.
 * @param x First vector.
 * @param y Second vector.
 * @return sqrt(sum((y - x) ^ 2))
 */
static STUPID_INLINE f32 stVec3Distance(const StVec3 x, const StVec3 y)
{
	return stSqrtf(stVec3Sum(stVec3Mul(stVec3Sub(y, x), stVec3Sub(y, x))));
}

/**
 * Calculates the distance between 2 StVec2s.
 * @param x First vector.
 * @param y Second vector.
 * @return sqrt(sum((y - x) ^ 2))
 */
static STUPID_INLINE f32 stVec2Distance(const StVec2 x, const StVec2 y)
{
	return stSqrtf(stVec2Sum(stVec2Mul(stVec2Sub(y, x), stVec2Sub(y, x))));
}

/**
 * Normalizes a StVec4.
 * @param vec Vector to normalize.
 * @return vec / hypot(vec)
 */
static STUPID_INLINE StVec4 stVec4Normalize(const StVec4 vec)
{
	const f32 hypot = stVec4Hypot(vec);
	return stVec4ScaleInv(vec, hypot);
}

/**
 * Normalizes a StVec3.
 * @param vec Vector to normalize.
 * @return vec / hypot(vec)
 */
static STUPID_INLINE StVec3 stVec3Normalize(const StVec3 vec)
{
	const f32 hypot = stVec3Hypot(vec);
	return stVec3ScaleInv(vec, hypot);
}

/**
 * Normalizes a StVec2.
 * @param vec Vector to normalize.
 * @return vec / hypot(vec)
 */
static STUPID_INLINE StVec2 stVec2Normalize(const StVec2 vec)
{
	const f32 hypot = stVec2Hypot(vec);
	return stVec2ScaleInv(vec, hypot);
}

/**
 * Returns a zerofilled StMat4.
 * @return {{0.0, 0.0, 0.0, 0.0},
 *          {0.0, 0.0, 0.0, 0.0},
 *          {0.0, 0.0, 0.0, 0.0},
 *          {0.0, 0.0, 0.0, 0.0}}
 */
static STUPID_INLINE StMat4 stMat4Zero(void)
{
	StMat4 mat;
	*((__m256*)(mat.m[0])) = _mm256_setzero_ps();
	*((__m256*)(mat.m[2])) = _mm256_setzero_ps();
	return mat;
}

/**
 * Returns a zerofilled StMat3.
 * @return {{0.0, 0.0, 0.0},
 *          {0.0, 0.0, 0.0},
 *          {0.0, 0.0, 0.0}}
 */
static STUPID_INLINE StMat3 stMat3Zero(void)
{
	StMat3 mat;
	*((__m256*)(mat.m[0])) = _mm256_setzero_ps();
	mat.m[2][2] = 0.0;
	return mat;
}

/**
 * Returns a zerofilled StMat2.
 * @return {{0.0, 0.0},
 *          {0.0, 0.0}}
 */
static STUPID_INLINE StMat2 stMat2Zero(void)
{
	StMat2 mat;
	*((__m128*)&mat) = _mm_setzero_ps();
	return mat;
}

/**
 * Returns a 4x4 identity matrix.
 * @return {{1.0, 0.0, 0.0, 0.0},
 *          {0.0, 1.0, 0.0, 0.0},
 *          {0.0, 0.0, 1.0, 0.0},
 *          {0.0, 0.0, 0.0, 1.0}}
 */
static STUPID_INLINE StMat4 stMat4Ident(void)
{
	StMat4 mat = stMat4Zero();
	mat.m[0][0] = 1.0;
	mat.m[1][1] = 1.0;
	mat.m[2][2] = 1.0;
	mat.m[3][3] = 1.0;
	return mat;
}

/**
 * Returns a 3x3 identity matrix.
 * @return {{1.0, 0.0, 0.0},
 *          {0.0, 1.0, 0.0},
 *          {0.0, 0.0, 1.0}}
 */
static STUPID_INLINE StMat3 stMat3Ident(void)
{
	StMat3 mat = stMat3Zero();
	mat.m[0][0] = 1.0;
	mat.m[1][1] = 1.0;
	mat.m[2][2] = 1.0;
	return mat;
}

/**
 * Returns a 2x2 identity matrix.
 * @return {{1.0, 0.0},
 *          {0.0, 1.0}}
 */
static STUPID_INLINE StMat2 stMat2Ident(void)
{
	StMat2 mat = stMat2Zero();
	mat.m[0][0] = 1.0;
	mat.m[1][1] = 1.0;
	return mat;
}

/**
 * Returns a new StMat4 with all elements set to the specified value.
 * @param value Value to set all elements of the new vector to.
 * @return {{value, value, value, value},
 *          {value, value, value, value},
 *          {value, value, value, value},
 *          {value, value, value, value}}
 */
static STUPID_INLINE StMat4 stMat4Set(const f32 value)
{
	StMat4 mat;
	const register __m256 b = _mm256_broadcast_ss(&value);
	*((__m256*)(mat.m[0])) = b;
	*((__m256*)(mat.m[2])) = b;
	return mat;
}

/**
 * Returns a new StMat3 with all elements set to the specified value.
 * @param value Value to set all elements of the new vector to.
 * @return {{value, value, value},
 *          {value, value, value},
 *          {value, value, value}},
 */
static STUPID_INLINE StMat3 stMat3Set(const f32 value)
{
	StMat3 mat;
	__m128 b128 = _mm_broadcast_ss(&value);
	__m256 b256 = _mm256_broadcastss_ps(b128);
	*((__m256*)(mat.m[0])) = b256;
	*((__m128*)(mat.m[2])) = b128;
	return mat;
}

/**
 * Returns a new StMat2 with all elements set to the specified value.
 * @param value Value to set all elements of the new vector to.
 * @return {{value, value},
 *          {value, value}},
 */
static STUPID_INLINE StMat2 stMat2Set(const f32 value)
{
	StMat2 mat;
	*((__m128*)&mat) = _mm_broadcast_ss(&value);
	return mat;
}

/**
 * Scales all elements in a StMat4 by the specified value.
 * @param mat Matrix to scale.
 * @param value Value to scale all elements in the specified matrix by.
 * @return {{mat.m[0][0] * value, mat.m[0][1] * value, mat.m[0][2] * value, mat.m[0][3] * value},
 *          {mat.m[1][0] * value, mat.m[1][1] * value, mat.m[1][2] * value, mat.m[1][3] * value},
 *          {mat.m[2][0] * value, mat.m[2][1] * value, mat.m[2][2] * value, mat.m[2][3] * value},
 *          {mat.m[3][0] * value, mat.m[3][1] * value, mat.m[3][2] * value, mat.m[3][3] * value}}
 */
static STUPID_INLINE StMat4 stMat4Scale(StMat4 mat, const f32 value)
{
	const register __m256 b = _mm256_broadcast_ss(&value);
	*((__m256*)(mat.m[0])) = _mm256_mul_ps(*((__m256*)(mat.m[0])), b);
	*((__m256*)(mat.m[2])) = _mm256_mul_ps(*((__m256*)(mat.m[2])), b);
	return mat;
}

/**
 * Scales all elements in a StMat3 by the specified value.
 * @param mat Matrix to scale.
 * @param value Value to scale all elements in the specified matrix by.
 * @return {{mat.m[0][0] * value, mat.m[0][1] * value, mat.m[0][2] * value},
 *          {mat.m[1][0] * value, mat.m[1][1] * value, mat.m[1][2] * value},
 *          {mat.m[2][0] * value, mat.m[2][1] * value, mat.m[2][2] * value}}
 */
static STUPID_INLINE StMat3 stMat3Scale(StMat3 mat, const f32 value)
{
	const register __m256 b = _mm256_broadcast_ss(&value);
	*((__m256*)(mat.m[0])) = _mm256_mul_ps(*((__m256*)(mat.m[0])), b);
	*((__m256*)(mat.m[2])) = _mm256_mul_ps(*((__m256*)(mat.m[2])), b);
	return mat;
}

/**
 * Scales all elements in a StMat2 by the specified value.
 * @param mat Matrix to scale.
 * @param value Value to scale all elements in the specified matrix by.
 * @return {{mat.m[0][0] * value, mat.m[0][1] * value},
 *          {mat.m[1][0] * value, mat.m[1][1] * value}}
 */
static STUPID_INLINE StMat2 stMat2Scale(StMat2 mat, const f32 x)
{
	__m128 b128 = _mm_broadcast_ss(&x);
	__m256 b256 = _mm256_broadcastss_ps(b128);
	*((__m256*)(mat.m[0])) = _mm256_mul_ps(*((__m256*)(mat.m[0])), b256);
	*((__m128*)(mat.m[2])) = _mm_mul_ps(*((__m128*)(mat.m[2])), b128);
	return mat;
}

/**
 * Swaps the rows and columns of a StMat4.
 * @param mat Matrix to transpose.
 * @return {{mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0]},
 *          {mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1]},
 *          {mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2]},
 *          {mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]}}
 */
static STUPID_INLINE StMat4 stMat4Transpose(const StMat4 mat)
{
	StMat4 res;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			res.m[i][j] = mat.m[j][i];
	return res;
}

/**
 * Swaps the rows and columns of a StMat3.
 * @param mat Matrix to transpose.
 * @return {{mat.m[0][0], mat.m[1][0], mat.m[2][0]},
 *          {mat.m[0][1], mat.m[1][1], mat.m[2][1]},
 *          {mat.m[0][2], mat.m[1][2], mat.m[2][2]}}
 */
static STUPID_INLINE StMat3 stMat3Transpose(const StMat3 mat)
{
	StMat3 res;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			res.m[i][j] = mat.m[j][i];
	return res;
}

/**
 * Swaps the rows and columns of a StMat3.
 * @param mat Matrix to transpose.
 * @return {{mat.m[0][0], mat.m[1][0]},
 *          {mat.m[0][1], mat.m[1][1]}}
 */
static STUPID_INLINE StMat2 stMat2Transpose(const StMat2 mat)
{
	StMat2 res;
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			res.m[i][j] = mat.m[j][i];
	return res;
}

/**
 * Multiplies one StMat4 by another.
 * @param x First matrix.
 * @param y Second matrix.
 * @return x * y
 * @note The order matters.
 */
static STUPID_INLINE StMat4 stMat4Mul(const StMat4 x, const StMat4 y)
{
	StMat4 res;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			res.m[i][j] = x.m[i][0] * y.m[0][j] +
				x.m[i][1] * y.m[1][j] +
				x.m[i][2] * y.m[2][j] +
				x.m[i][3] * y.m[3][j];
	return res;
}

/**
 * Multiplies one StMat3 by another.
 * @param x First matrix.
 * @param y Second matrix.
 * @return x * y
 * @note The order matters.
 */
static STUPID_INLINE StMat3 stMat3Mul(const StMat3 x, const StMat3 y)
{
	StMat3 res;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			res.m[i][j] = x.m[i][0] * y.m[0][j] +
				x.m[i][1] * y.m[1][j] +
				x.m[i][2] * y.m[2][j];
	return res;
}

/**
 * Multiplies one StMat2 by another.
 * @param x First matrix.
 * @param y Second matrix.
 * @return x * y
 * @note The order matters.
 */
static STUPID_INLINE StMat2 stMat2Mul(const StMat2 x, const StMat2 y)
{
	StMat2 res;
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			res.m[i][j] = x.m[i][0] * y.m[0][j] +
				x.m[i][1] * y.m[1][j];
	return res;
}

/**
 * Adds two StMat4s.
 * @param x First matrix.
 * @param y Second matrix.
 * @return {{x[0][0] + y[0][0], x[0][1] + y[0][1], x[0][2] + y[0][2], x[0][3] + y[0][3]},
 *          {x[1][0] + y[1][0], x[1][1] + y[1][1], x[1][2] + y[1][2], x[1][3] + y[1][3]},
 *          {x[2][0] + y[2][0], x[2][1] + y[2][1], x[2][2] + y[2][2], x[2][3] + y[2][3]},
 *          {x[3][0] + y[3][0], x[3][1] + y[3][1], x[3][2] + y[3][2], x[3][3] + y[3][3]}}
 */
static STUPID_INLINE StMat4 stMat4Add(const StMat4 x, const StMat4 y)
{
	StMat4 res;
	*((__m256*)res.m[0]) = _mm256_add_ps(*((__m256*)x.m[0]), *((__m256*)y.m[0]));
	*((__m256*)res.m[2]) = _mm256_add_ps(*((__m256*)x.m[2]), *((__m256*)y.m[2]));
	return res;
}

/**
 * Adds two StMat3s.
 * @param x First matrix.
 * @param y Second matrix.
 * @return {{x[0][0] + y[0][0], x[0][1] + y[0][1], x[0][2] + y[0][2]},
 *          {x[1][0] + y[1][0], x[1][1] + y[1][1], x[1][2] + y[1][2]}, {x[2][0] + y[2][0], x[2][1] + y[2][1], x[2][2] + y[2][2]}}
 */
static STUPID_INLINE StMat3 stMat3Add(const StMat3 x, const StMat3 y, StMat3 result)
{
	StMat3 res;
	*((__m256*)res.m[0]) = _mm256_add_ps(*((__m256*)x.m[0]), *((__m256*)y.m[0]));
	*((__m128*)res.m[2]) = _mm_mul_ps(*((__m128*)x.m[2]), *((__m128*)y.m[2]));
	return res;
}

/**
 * Adds two StMat2s.
 * @param x First matrix.
 * @param y Second matrix.
 * @return {{x[0][0] + y[0][0], x[0][1] + y[0][1]},
 *          {x[1][0] + y[1][0], x[1][1] + y[1][1]}}
 */
static STUPID_INLINE StMat2 stMat2Add(const StMat2 x, const StMat2 y)
{
	StMat2 res;
	*((__m128*)&res) = _mm_add_ps(*((__m128*)&x), *((__m128*)&y));
	return res;
}

/**
 * Subtracts one StMat4 from another.
 * @param x First matrix.
 * @param y Second matrix.
 * @return {{x[0][0] - y[0][0], x[0][1] - y[0][1], x[0][2] - y[0][2], x[0][3] - y[0][3]},
 *          {x[1][0] - y[1][0], x[1][1] - y[1][1], x[1][2] - y[1][2], x[1][3] - y[1][3]},
 *          {x[2][0] - y[2][0], x[2][1] - y[2][1], x[2][2] - y[2][2], x[2][3] - y[2][3]},
 *          {x[3][0] - y[3][0], x[3][1] - y[3][1], x[3][2] - y[3][2], x[3][3] - y[3][3]}}
 */
static STUPID_INLINE StMat4 stMat4Sub(const StMat4 x, const StMat4 y)
{
	StMat4 res;
	*((__m256*)res.m[0]) = _mm256_sub_ps(*((__m256*)x.m[0]), *((__m256*)y.m[0]));
	*((__m256*)res.m[2]) = _mm256_sub_ps(*((__m256*)x.m[2]), *((__m256*)y.m[2]));
	return res;
}

/**
 * Subtracts one StMat3 from another.
 * @param x First matrix.
 * @param y Second matrix.
 * @return {{x[0][0] - y[0][0], x[0][1] - y[0][1], x[0][2] - y[0][2]},
 *          {x[1][0] - y[1][0], x[1][1] - y[1][1], x[1][2] - y[1][2]},
 *          {x[2][0] - y[2][0], x[2][1] - y[2][1], x[2][2] - y[2][2]}}
 */
static STUPID_INLINE StMat3 stMat3Sub(const StMat3 x, const StMat3 y, StMat3 result)
{
	StMat3 res;
	*((__m256*)res.m[0]) = _mm256_sub_ps(*((__m256*)x.m[0]), *((__m256*)y.m[0]));
	*((__m128*)res.m[2]) = _mm_sub_ps(*((__m128*)x.m[2]), *((__m128*)y.m[2]));
	return res;
}

/**
 * Subtracts one StMat2 from another.
 * @param x First matrix.
 * @param y Second matrix.
 * @return {{x[0][0] + y[0][0], x[0][1] + y[0][1]},
 *          {x[1][0] + y[1][0], x[1][1] + y[1][1]}}
 */
static STUPID_INLINE StMat2 stMat2Sub(const StMat2 x, const StMat2 y)
{
	StMat2 res;
	*((__m128*)&res) = _mm_sub_ps(*((__m128*)&x), *((__m128*)&y));
	return res;
}

/**
 * Gets the determinant of a StMat2.
 * @param mat Matrix to get the determinant of.
 * @return det(mat)
 * @note This is somewhat useless.
 */
static STUPID_INLINE f32 stMat2Det(const StMat2 mat)
{
	return mat.m[0][0] * mat.m[1][1] - mat.m[0][1] * mat.m[1][0];
}

/**
 * Gets the determinant of a StMat3.
 * @param mat Matrix to get the determinant of.
 * @return det(mat)
 * @note This is somewhat useless.
 */
static STUPID_INLINE f32 stMat3Det(const StMat3 mat)
{
	const f32 x = stMat2Det((StMat2){
		.m = {
			{mat.m[1][1], mat.m[1][2]},
			{mat.m[2][1], mat.m[2][2]}
		}
	});

	const f32 y = stMat2Det((StMat2){
		.m = {
			{mat.m[1][0], mat.m[1][2]},
			{mat.m[2][0], mat.m[2][2]}
		}
	});

	const f32 z = stMat2Det((StMat2){
		.m = {
			{mat.m[1][0], mat.m[1][1]},
			{mat.m[2][0], mat.m[2][1]}
		}
	});

	return (mat.m[0][0] * x) - (mat.m[0][1] * y) + (mat.m[0][2] * z);
}

/**
 * Gets the determinant of a StMat4.
 * @param mat Matrix to get the determinant of.
 * @return det(mat)
 * @note This is somewhat useless.
 */
static inline f32 stMat4Det(const StMat4 mat)
{
	const f32 x = stMat3Det((StMat3){
		.m = {
			{mat.m[1][1], mat.m[1][2], mat.m[1][3]},
			{mat.m[2][1], mat.m[2][2], mat.m[2][3]},
			{mat.m[3][1], mat.m[3][2], mat.m[3][3]}
		}
	});

	const f32 y = stMat3Det((StMat3){
		.m = {
			{mat.m[1][0], mat.m[1][2], mat.m[1][3]},
			{mat.m[2][0], mat.m[2][2], mat.m[2][3]},
			{mat.m[3][0], mat.m[3][2], mat.m[3][3]}
		}
	});

	const f32 z = stMat3Det((StMat3){
		.m = {
			{mat.m[1][0], mat.m[1][1], mat.m[1][3]},
			{mat.m[2][0], mat.m[2][1], mat.m[2][3]},
			{mat.m[3][0], mat.m[3][1], mat.m[3][3]}
		}
	});

	const f32 w = stMat3Det((StMat3){
		.m = {
			{mat.m[1][0], mat.m[1][1], mat.m[1][2]},
			{mat.m[2][0], mat.m[2][1], mat.m[2][2]},
			{mat.m[3][0], mat.m[3][1], mat.m[3][2]}
		}
	});

	return (mat.m[0][0] * x) - (mat.m[0][1] * y) + (mat.m[0][2] * z) - (mat.m[0][3] * w);
}

/**
 * Creates an orthographic view matrix.
 * @param left Left clip.
 * @param right Right clip.
 * @param bottom Bottom clip.
 * @param top Top clip.
 * @param near Near clip.
 * @param far Far clip.
 * @return An orthographic view matrix based on the specified parameters.
 * @note Your probably looking for stMat4Perspective instead.
 */
static STUPID_INLINE StMat4 stMat4Orthographic(const f32 left, const f32 right, const f32 bottom, const f32 top,
                                        const f32 near, const f32 far)
{
	StMat4 mat = stMat4Ident();

	mat.m[0][0] = 2.0f * (right * left);
	mat.m[1][1] = -2.0f / (top - bottom);
	mat.m[2][2] = 1.0f * (far - near);
	mat.m[3][0] = -(right + left) / (right - left);
	mat.m[3][1] = (top + bottom) / (top - bottom);
	mat.m[3][2] = -near / (far - near);

	return mat;
}

/**
 * Creates a perspective projection view matrix.
 * @param fov Field of view in radians.
 * @param aspect_ratio Screen width / screen height.
 * @param near Near clip.
 * @param far Far clip.
 * @return A perspedtive view matrix (basically a camera).
 */
static STUPID_INLINE StMat4 stMat4Perspective(const f32 fov, const f32 aspect_ratio, const f32 near, const f32 far)
{
	StMat4 mat = stMat4Zero();
	const f32 scale = stTan(fov * 0.5f);

	mat.m[0][0] = 1.0f / (aspect_ratio * scale);
	mat.m[1][1] = -(1.0f / scale);
	mat.m[2][2] = far / (far - near);
	mat.m[2][3] = -((far * near) / (far - near));
	mat.m[3][2] = 1.0f;

	return mat;
}

static STUPID_INLINE StMat4 stMat4LookAt(StVec3 pos, const StVec3 target, const StVec3 up)
{
	const StVec3 f = stVec3Normalize(stVec3Sub(target, pos));
	const StVec3 r = stVec3Normalize(stVec3Cross(up, f));
	const StVec3 u = stVec3Normalize(stVec3Cross(f, r));

	StMat4 mat = stMat4Ident();

	mat.m[0][0] = r.x;
	mat.m[0][1] = r.y;
	mat.m[0][2] = r.z;
	mat.m[0][3] = -(stVec3Dot(r, pos));

	mat.m[1][0] = u.x;
	mat.m[1][1] = u.y;
	mat.m[1][2] = u.z;
	mat.m[1][3] = -(stVec3Dot(u, pos));

	mat.m[2][0] = f.x;
	mat.m[2][1] = f.y;
	mat.m[2][2] = f.z;
	mat.m[2][3] = -(stVec3Dot(f, pos));

	return mat;
}

/**
 * Creates an x axis rotation matrix from an angle.
 * @param theta An angle to rotate along the x axis.
 * @return A matrix for rotating a vector along the x axis by theta.
 */
static STUPID_INLINE StMat4 stMat4EulerX(const f32 theta)
{
	StMat4 mat = stMat4Ident();

	const f32 sin = stSin(theta);
	const f32 cos = stCos(theta);

	mat.m[1][1] = cos;
	mat.m[1][2] = sin;
	mat.m[2][1] = -sin;
	mat.m[2][2] = cos;

	return mat;
}

/**
 * Creates a y axis rotation matrix from an angle.
 * @param theta An angle to rotate along the y axis.
 * @return A matrix for rotating a vector along the y axis by theta.
 */
static STUPID_INLINE StMat4 stMat4EulerY(const f32 theta)
{
	StMat4 mat = stMat4Ident();

	const f32 sin = stSin(theta);
	const f32 cos = stCos(theta);

	mat.m[0][0] = cos;
	mat.m[0][2] = -sin;
	mat.m[2][0] = sin;
	mat.m[2][2] = cos;

	return mat;
}

/**
 * Creates a z axis rotation matrix from an angle.
 * @param theta An angle to rotate along the z axis.
 * @return A matrix for rotating a vector along the z axis by theta.
 */
static STUPID_INLINE StMat4 stMat4EulerZ(const f32 theta)
{
	StMat4 mat = stMat4Ident();

	const f32 sin = stSin(theta);
	const f32 cos = stCos(theta);

	mat.m[0][0] = cos;
	mat.m[0][1] = sin;
	mat.m[1][0] = -sin;
	mat.m[1][1] = cos;

	return mat;
}

/**
 * Creates an x axis rotation matrix from an angle.
 * @param theta An angle to rotate along the x axis.
 * @return A matrix for rotating a vector along the x axis by theta.
 */
static STUPID_INLINE StMat3 stMat3EulerX(const f32 theta)
{
	StMat3 mat = stMat3Ident();

	const f32 sin = stSin(theta);
	const f32 cos = stCos(theta);

	mat.m[1][1] = cos;
	mat.m[1][2] = sin;
	mat.m[2][1] = -sin;
	mat.m[2][2] = cos;

	return mat;
}

/**
 * Creates a y axis rotation matrix from an angle.
 * @param theta An angle to rotate along the y axis.
 * @return A matrix for rotating a vector along the y axis by theta.
 */
static STUPID_INLINE StMat3 stMat3EulerY(const f32 theta)
{
	StMat3 mat = stMat3Ident();

	const f32 sin = stSin(theta);
	const f32 cos = stCos(theta);

	mat.m[0][0] = cos;
	mat.m[0][2] = -sin;
	mat.m[2][0] = sin;
	mat.m[2][2] = cos;

	return mat;
}

/**
 * Creates a z axis rotation matrix from an angle.
 * @param theta An angle to rotate along the z axis.
 * @return A matrix for rotating a vector along the z axis by theta.
 */
static STUPID_INLINE StMat3 stMat3EulerZ(const f32 theta)
{
	StMat3 mat = stMat3Ident();

	const f32 sin = stSin(theta);
	const f32 cos = stCos(theta);

	mat.m[0][0] = cos;
	mat.m[0][1] = sin;
	mat.m[1][0] = -sin;
	mat.m[1][1] = cos;

	return mat;
}

/**
 * Creates a rotation matrix from 3 angles, and 3 translation angles.
 * @param mat Output matrix.
 * @param angle Angle to spin clockwise by on the specified axis.
 * @param axis Pivot angle to rotate to.
 * @return A rotation matrix based on the specified angle and axis.
 */
static inline StMat3 stMat3Rotate(const f32 angle, const StVec3 axis)
{
	const f32 s = stSin(angle);
	const f32 c = stCos(angle);
	const f32 t = 1.0f - c;

	const f32 magnitude = stVec3Hypot(axis);
	if (magnitude < 0.0000001)
		return stMat3Ident();

	StVec3 norm = stVec3ScaleInv(axis, magnitude);

	const f32 x = norm.v[0];
	const f32 y = norm.v[1];
	const f32 z = norm.v[2];

	const f32 tx = t * x;
	const f32 ty = t * y;

	StMat3 mat;

	mat.m[0][0] = tx * x + c;
	mat.m[0][1] = tx * y - z * s;
	mat.m[0][2] = tx * z + y * s;

	mat.m[1][0] = tx * y + z * s;
	mat.m[1][1] = ty * y + c;
	mat.m[1][2] = ty * z + x * s;

	mat.m[2][0] = tx * z - y * s;
	mat.m[2][1] = ty * z - x * s;
	mat.m[2][2] = t * z * z + c;

	return mat;
}

/**
 * Creates a rotation matrix from 3 angles, and 3 translation angles.
 * @param angle Angle to spin clockwise by on the specified axis.
 * @param axis Pivot angle to rotate to.
 * @param translation Translation
 * @return A rotation matrix based on the specified angle and axis.
 */
static inline StMat4 stMat4Rotate(const f32 angle, const StVec3 axis, const StVec3 translation)
{
	const f32 s = stSin(angle);
	const f32 c = stCos(angle);
	const f32 t = 1.0f - c;

	const f32 magnitude = stVec3Hypot(axis);
	if (magnitude < 0.0000001)
		return stMat4Ident();

	StVec3 norm = stVec3ScaleInv(axis, magnitude);

	const f32 x = norm.v[0];
	const f32 y = norm.v[1];
	const f32 z = norm.v[2];

	const f32 tx = t * x;
	const f32 ty = t * y;

	StMat4 mat;

	mat.m[0][0] = tx * x + c;
	mat.m[0][1] = tx * y - z * s;
	mat.m[0][2] = tx * z + y * s;
	mat.m[0][3] = translation.v[0];

	mat.m[1][0] = tx * y + z * s;
	mat.m[1][1] = ty * y + c;
	mat.m[1][2] = ty * z + x * s;
	mat.m[1][3] = translation.v[1];

	mat.m[2][0] = tx * z - y * s;
	mat.m[2][1] = ty * z - x * s;
	mat.m[2][2] = t * z * z + c;
	mat.m[2][3] = translation.v[2];

	*((__m128*)mat.m[3]) = _mm_setzero_ps();
	mat.m[3][3] = 1.0;
	return mat;
}

static STUPID_INLINE StMat4 stMat4Scaler(const f32 x, const f32 y, const f32 z)
{
	StMat4 mat = stMat4Zero();

	mat.m[0][0] = x;
	mat.m[1][1] = y;
	mat.m[2][2] = z;

	return mat;
}

static STUPID_INLINE StMat4 stMat4Translate(StMat4 mat, const f32 x, const f32 y, const f32 z)
{
	mat.m[0][3] = x;
	mat.m[1][3] = y;
	mat.m[2][3] = z;

	return mat;
}

/// Multiplies a vector by a matrix.
/// @note res and vec cannot overlap.
static STUPID_INLINE StVec4 stVec4MulMat(const StVec4 vec, const StMat4 mat)
{
	StVec4 res = stVec4Zero();
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			res.v[i] += mat.m[i][j] * vec.v[j];
	return res;
}

/// Multiplies a vector by a matrix.
/// @note res and vec cannot overlap.
static STUPID_INLINE StVec3 stVec3MulMat(const StVec3 vec, const StMat3 mat)
{
	StVec3 res = stVec3Zero();
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			res.v[i] += mat.m[i][j] * vec.v[j];
	return res;
}

/// Multiplies a vector by a matrix.
/// @note res and vec cannot overlap.
static STUPID_INLINE StVec2 stVec2MulMat(const StVec2 vec, const StMat2 mat)
{
	StVec2 res = stVec2Zero();
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			res.v[i] += mat.m[i][j] * vec.v[j];
	return res;
}

// "Have you guessed the riddle yet?" the Hatter said, turning to Alice again.
// "No, I give it up," Alice replied: "What's the answer?"
// "I haven't the slightest idea." said the Hatter.

static STUPID_INLINE StQuat stQuatConjugate(const StQuat quat)
{
	StQuat res = quat;
	res.v = stVec3Scale(res.v, -1.0f);
	return res;
}

static STUPID_INLINE StQuat stQuatNormalize(StQuat quat)
{
	StVec4 vec;
	vec.x = quat.w;
	vec.y = quat.x;
	vec.z = quat.y;
	vec.w = quat.z;

	vec = stVec4Normalize(vec);

	quat.w = vec.x;
	quat.x = vec.y;
	quat.y = vec.z;
	quat.z = vec.w;

	return quat;
}

static STUPID_INLINE StVec3 stVec3MulQuat(const StVec3 vec, const StQuat quat)
{
	StVec3 res = stVec3Scale(stVec3Cross(quat.v, vec), 2.0f);
	res = stVec3Add(vec, stVec3Add(stVec3Scale(res, quat.w), stVec3Cross(quat.v, res)));
	return res;
}



















