/// @file memory.h
/// @brief Provides functions for memory allocation, reallocation, and deallocation.
/// This includes functions for allocating memory, and other types of functions like memcpy and memset.
/// @author nonexistant

#pragma once

#include "stupid/common.h"
#include "stupid/assert.h"

typedef struct STUPID_A8 StMemory {
        usize   stride;
        usize   length;
        usize   capacity;
        char    *type_name;
} StMemory;

#define ST_MEMORY_HEADER_SIZE sizeof(StMemory)

#define ST_MEMORY_CAST(x) ((StMemory *)(((u8 *)(x)) - ST_MEMORY_HEADER_SIZE))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NASM AVX2 forward memcpy.
 * Copies n bytes from src to dest.
 * @param dest Destination buffer which MUST have a size >= n.
 * @param src Source buffer which MUST have a size >= n.
 * @param n Number of bytes to copy.
 * @return dest.
 * @note Only works correctly if dest is less than src.
 */
extern void *__stCpyFwd(void *dest, const void *src, const usize n);

/**
 * @brief NASM AVX2 backward memcpy.
 * Copies n bytes from src to dest.
 * @param dest Destination buffer which MUST have a size >= n.
 * @param src Source buffer which MUST have a size >= n.
 * @param n Number of bytes to copy.
 * @return dest.
 * @note Only works correctly if src is more than src.
 */
extern void *__stCpyBkwd(void *dest, const void *src, const usize n);

/**
 * @brief NASM AVX2 memset.
 * Sets all n bytes of dest to c.
 * @param dest Destination buffer which MUST have a size >= n.
 * @param c Value to set bytes to.
 * @param n Number of bytes to set.
 * @return dest.
 */
extern void *stMemset(void *dest, char c, usize n);

/**
 * NASM AVX2 memcmp-like function.
 * Checks if the first n bytes of p1 and p2 are the same.
 * @param p1 First buffer which MUST have a size >= n.
 * @param p2 Second pointer which MUST have a size >= n.
 * @param n Number of bytes to check.
 * @return True if the first n bytes of p1 and p2 are the same.
 * @note Equivalent to (memcmp(p1, p2, n) == 0).
 */
extern bool stMemeq(const void *p1, const void *p2, usize n);

#ifdef __cplusplus
}
#endif

void stMemUsage(void);

/**
 * @brief NASM AVX2 memcpy.
 * Copies n bytes from src to dest.
 * @param dest Destination buffer which MUST have a size >= n.
 * @param src Source buffer which MUST have a size >= n.
 * @param n Number of bytes to copy.
 * @return dest.
 * @note If dest and src overlap, then data will not be correctly copied.
 */
static STUPID_INLINE void *stMemcpy(void *dest, const void *src, const usize n)
{
        return __stCpyBkwd(dest, src, n);
}

/**
 * @brief NASM AVX2 memmove.
 * Copies n bytes from src to dest in a way where dest and src can overlap.
 * @param dest Destination buffer which MUST have a size >= n.
 * @param src Source buffer which MUST have a size >= n.
 * @param n Number of bytes to copy.
 * @return dest.
 */
static STUPID_INLINE void *stMemMove(void *dest, const void *src, const usize n)
{
        if (dest > src)
                return __stCpyBkwd(dest, src, n);
        else if (dest == src)
                return dest;
        else
                return __stCpyFwd(dest, src, n);
}

/**
 * @brief Creates an array.
 * The resulting array stores the number of elements, stride, capacity, and type name.
 * @param stride The size of each element.
 * @param capacity The number of elements to allocate space for.
 * @param type_name The name of the type this array will hold.
 * @return The new array.
 * @note Allocations are always aligned to 32 bytes.
 * @note Automatically zerofills memory.
 */
void STUPID_ATTR_MALLOC *(stMemAlloc)(const usize stride, const usize capacity, char *type_name STUPID_DBG_PROTO_PARAMS);

/**
 * @brief Creates an array.
 * The resulting array stores the number of elements, stride, capacity, and type name.
 * @param capacity The number of elements to allocate space for.
 * @return The new array.
 * @note Allocations are always aligned to 32 bytes.
 * @note Automatically zerofills memory.
 */
#define stMemAlloc(type, capacity) (stMemAlloc)(sizeof(type), capacity, #type STUPID_DBG_PARAMS)

/**
 * @brief Creates an array.
 * The resulting array stores the number of elements, stride, capacity, and type name.
 * @param capacity The number of elements to allocate space for.
 * @return The new array.
 * @note Allocations are always aligned to 32 bytes.
 * @note Automatically zerofills memory.
 * @note Does not print logs.
 */
#define stMemAllocNL(type, capacity) (stMemAlloc)(sizeof(type), capacity, #type STUPID_DBG_PARAMS_NL)

/**
 * Equivalent to stMemAlloc(char, size) except for the type name.
 * @param size The number of bytes to allocate.
 * @return The new array.
 * @note Allocations are always aligned to 32 bytes.
 * @note Automatically zerofills memory.
 */
#define stMemAllocs(size) (stMemAlloc)(size, 1, "byte" STUPID_DBG_PARAMS)

/**
 * Equivalent to stMemAlloc(char, size) except for the type name.
 * @param size The number of bytes to allocate.
 * @return The new array.
 * @note Allocations are always aligned to 32 bytes.
 * @note Automatically zerofills memory.
 * @note Does not print logs.
 */
#define stMemAllocsNL(size) (stMemAlloc)(size, 1, "byte"  STUPID_DBG_PARAMS_NL)

/**
 * Resizes an array.
 * @param array Pointer to an array created with stMemAlloc().
 * @param new_capacity The number of elements the new array will be able to hold.
 * @return The same array resized to capacity.
 * @note This does not use realloc(), but instead allocates a new
 * array, and moves the old one to the new one so it will keep its alignment.
 * @note Avoid resizing down unless you can be sure you wont need to resize up at all or at least for a while.
 * @note This becomes expensive with larger arrays since it has to copy the entire array to the new one.
 */
void STUPID_ATTR_MALLOC *(stMemResize)(void **array, const usize new_capacity STUPID_DBG_PROTO_PARAMS);

/**
 * Resizes an array.
 * @param array An array created with stMemAlloc().
 * @param capacity The number of elements the new array will be able to hold.
 * @return The same array resized to capacity.
 * @note This does not use realloc(), but instead allocates a new
 * array, and moves the old one to the new one so it will keep its alignment.
 * @note Avoid resizing down unless you can be sure you wont need to resize up at all or at least for a while.
 * @note This becomes expensive with larger arrays since it has to copy the entire array to the new one.
 */
#define stMemResize(array, capacity)   (stMemResize)((void **)&(array), capacity STUPID_DBG_PARAMS)

/**
 * Resizes an array.
 * @param array an array created with stMemAlloc()
 * @param capacity the number of elements the new array will be able to hold
 * @return The same array resized to capacity.
 * @note This does not use realloc(), but instead allocates a new
 * array, and moves the old one to the new one so it will keep its alignment.
 * @note Avoid resizing down unless you can be sure you wont need to resize up at all or at least for a while.
 * @note This becomes expensive with larger arrays since it has to copy the entire array to the new one.
 * @note Does not print logs.
 */
#define stMemResizeNL(array, capacity) (stMemResize)((void **)&(array), capacity STUPID_DBG_PARAMS_NL)

/**
 * Deallocates an array.
 * @param array A pointer to an array created with stMemAlloc().
 */
void (stMemDealloc)(void **array STUPID_DBG_PROTO_PARAMS);

/**
 * Deallocates an array.
 * @param array A pointer to an array created with stMemAlloc().
 */
#define stMemDealloc(array)   (stMemDealloc)((void **)&(array) STUPID_DBG_PARAMS)

/**
 * Deallocates an array.
 * @param array A pointer to an array created with stMemAlloc().
 * @note Does not print logs.
 */
#define stMemDeallocNL(array) (stMemDealloc)((void **)&(array) STUPID_DBG_PARAMS_NL)

/**
 * Appends an element to the end of an array, and increments the length.
 * @param array A pointer to an array created with stMemAlloc().
 * @param data A pointer to the element.
 */
void (stMemAppend)(void **array, const void *data);

/**
 * Appends an element to the end of an array, and increments the length.
 * @param array An array created with stMemAlloc().
 * @param item A value of the same type as the array.
 * @todo Fix clangd warning from static assertion.
 */
#define stMemAppend(array, item)\
        do {\
		STUPID_STATIC_ASSERT(sizeof(item) == sizeof(*(array)), "cannot append '" #item "' (invalid type)");\
                __typeof__((item)) x = (item);\
                (stMemAppend)((void **)&(array), &x);\
        } while (0)

/**
 * Inserts an element anywhere within an array's bounds.
 * @param array A pointer to an array created with stMemAlloc().
 * @param position The position to insert the element at (0 based obviously).
 * @param data A pointer to the element.
 * @note position must be less than the number of elements in the array.
 * @note Moves elements forward to make room for the new element instead of replacing an element.
 */
void (stMemInsert)(void **array, const usize position, const void *data STUPID_DBG_PROTO_PARAMS);

/**
 * Inserts an element anywhere within an array's bounds.
 * @param array An array created with stMemAlloc().
 * @param position The position to insert the element at (0 based obviously).
 * @param item A value of the same type as the array.
 * @note position must be less than the number of elements in the array.
 * @note Moves elements forward to make room for the new element instead of replacing an element.
 */
#define stMemInsert(array, position, item)\
        do {\
                __typeof__(item) x = item;\
                (stMemInsert)((void **)&(array), position, &x STUPID_DBG_PARAMS);\
        } while (0)

/**
 * Inserts an element anywhere within an array's bounds.
 * @param array An array created with stMemAlloc().
 * @param position The position to insert the element at (0 based obviously).
 * @param item A value of the same type as the array.
 * @note position must be less than the number of elements in the array.
 * @note Moves elements forward to make room for the new element instead of replacing an element.
 * @note Does not print logs.
 */
#define stMemInsertNL(array, position, item)\
        do {\
                __typeof__(item) x = item;\
                (stMemInsert)((void **)&(array), position, &x STUPID_DBG_PARAMS_NL);\
        } while (0)

/**
 * Removes an element from an array, and copies it to output if output isnt NULL.
 * @param array A pointer to an array created with stMemAlloc().
 * @param position The position of the element to remove (0 based obviously).
 * @param output A pointer to copy the element being removed to (can be NULL).
 * @note position must be less than the number of elements in the array.
 */
void (stMemRemove)(void **array, const usize position, void *output STUPID_DBG_PROTO_PARAMS);

/**
 * Removes an element from an array, and copies it to output if output isnt NULL.
 * @param array an array created with stMemAlloc()
 * @param position The position of the element to remove (0 based obviously).
 * @param output A pointer to copy the element being removed to (can be NULL).
 * @note position must be less than the number of elements in the array.
 */
#define stMemRemove(array, position, output) (stMemRemove)((void **)&(array), position, output STUPID_DBG_PARAMS)

/**
 * Removes an element from an array, and copies it to output if output isnt NULL.
 * @param array an array created with stMemAlloc()
 * @param position The position of the element to remove (0 based obviously).
 * @param output A pointer to copy the element being removed to (can be NULL).
 * @note position must be less than the number of elements in the array.
 * @note Does not print logs.
 */
#define stMemRemoveNL(array, position, output) (stMemRemove)((void **)&(array), position, output STUPID_DBG_PARAMS_NL)

/**
 * Removes the last element from an array, and copies it to output if output isnt NULL.
 * @param array An array created with stMemAlloc().
 * @param output A pointer to copy the element being removed to (can be NULL).
 */
#define stMemPop(array, output)   stMemRemove(array, (stMemLength(array) > 0 ? stMemLength(array) - 1 : 0), output STUPID_DBG_PARAMS)

/**
 * Removes the last element from an array, and copies it to output if output isnt NULL.
 * @param array An array created with stMemAlloc().
 * @param output A pointer to copy the element being removed to (can be NULL).
 * @note Does not print logs.
 */
#define stMemPopNL(array, output) stMemRemove(array, (stMemLength(array) > 0 ? stMemLength(array) - 1 : 0), output STUPID_DBG_PARAMS_NL)

/**
 * Gets the size of an array in bytes.
 * @param array An array created with stMemAlloc().
 * @return The size of each element multiplied by the number of elements.
 */
static STUPID_INLINE usize stMemSize(void *array)
{
        STUPID_NC(array);
        return ST_MEMORY_CAST(array)->length * ST_MEMORY_CAST(array)->stride;
}

/**
 * Gets the size of each element in an array.
 * @param array An array created with stMemAlloc().
 * @return The size of each element in the array.
 */
static STUPID_INLINE usize stMemStride(void *array)
{
        STUPID_NC(array);
        return ST_MEMORY_CAST(array)->stride;
}

/**
 * Gets the number of elements in an array.
 * @param array An array created with stMemAlloc().
 * @return The number of elements in an array.
 * @note This only counts elements inserted with things like stMemAppend() or stMemInsert().
 */
static STUPID_INLINE usize stMemLength(void *array)
{
        STUPID_NC(array);
        return ST_MEMORY_CAST(array)->length;
}

/**
 * Gets the capacity of an array.
 * @param array An array created with stMemAlloc().
 * @return The number of elements an array can hold.
 */
static STUPID_INLINE usize stMemCapacity(void *array)
{
        STUPID_NC(array);
        return ST_MEMORY_CAST(array)->capacity;
}

/**
 * Sets the number of elements in an array.
 * @param array An array created with stMemAlloc().
 */
static STUPID_INLINE void stMemSetLength(void *array, const usize length)
{
        STUPID_NC(array);
        ST_MEMORY_CAST(array)->length = length;
}

/**
 * Increments the length of an array by 1.
 * @param array An array created with stMemAlloc().
 * @note Equivalent to stMemSetLength(array, stMemLength(array) + 1).
 */
static STUPID_INLINE void stMemIncrementLength(void *array)
{
        STUPID_NC(array);
        ST_MEMORY_CAST(array)->length = ST_MEMORY_CAST(array)->length + 1;
}

/**
 * Duplicates an array.
 * @param array An array created with stMemAlloc().
 * @return A copy of the array.
 */
static STUPID_INLINE void *stMemDupe(void *array)
{
        STUPID_NC(array);
        const StMemory *mem = ST_MEMORY_CAST(array);
        u8 *new_array = (stMemAlloc)(mem->stride, mem->capacity, mem->type_name STUPID_DBG_PARAMS_NL);
        new_array += ST_MEMORY_HEADER_SIZE;
        return stMemcpy(new_array, array, stMemSize(array));
}

/**
 * Gets the length of a string.
 * @param s Input string.
 * @param n Max length.
 */
static STUPID_INLINE usize stStrlen(const char *s, usize n)
{
	usize start = (usize)s;
	while (*s && n--) s++;
	return (usize)s - (usize)start;
}

static STUPID_INLINE bool stStrneq(const char *str1, const char *str2, usize n)
{
        while (*str1 && *str2 && n--)
                if (*str1++ != *str2++) return false;

        return true;
}

static STUPID_INLINE i64 stFind(const void *array, const void *element, const usize stride, const usize length)
{
	for (int i = 0; i < length; i++) {
		if (stMemeq(array, element, stride)) return i;
		array += stride;
	}

	return -1;
}

