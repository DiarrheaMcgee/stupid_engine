#include <stdlib.h>
#define MALLOC_IMPL  malloc
#define ALIGNED_IMPL aligned_alloc
#define REALLOC_IMPL realloc
#define FREE_IMPL    free

#include "stupid/memory.h"
#include "stupid/assert.h"
#include "stupid/logger.h"
#include "stupid/math/basic.h"

#define MEMORY_TAG_COUNT 256

typedef struct STUPID_A32 MemoryStats {
	usize tag[MEMORY_TAG_COUNT];
	usize total;
	const char *type_name[MEMORY_TAG_COUNT];
} MemoryStats;

static MemoryStats stats = {0};
MemoryStats stats2 = {0};

static usize typeNameToIndex(const char *type_name)
{
	for (int i = 0; i < MEMORY_TAG_COUNT; i++) {
		if (stats.type_name[i] == NULL) {
			stats.type_name[i] = type_name;
			return i;
		}
		else if (type_name == stats.type_name[i]) return i;
	}

	return 0;
}

void stMemUsage(void)
{
	STUPID_LOG_INFO("total: %zu", stats.total);

	for (usize i = 0; i < MEMORY_TAG_COUNT; i++) {
		if (stats.tag[i] == 0) continue;
		STUPID_LOG_INFO("%s: %zu", stats.type_name[i], stats.tag[i]);
	}
}

void STUPID_ATTR_MALLOC *(stMemAlloc)(const usize stride, const usize capacity, char *type_name STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(type_name);

	STUPID_ASSERT(stride != 0, "this is not valid");

	u8 *data = NULL;

	// the number of bytes to allocate
	usize size = (stride * capacity + ST_MEMORY_HEADER_SIZE);

	// round it up to the nearest multiple of 32
	size = (size + 31) & (-32);

	// allocate the array with 32 byte alignment
	data = ALIGNED_IMPL(32, size);

	// TODO: check if an assertion would be better
	if (STUPID_UNLIKELY(data == NULL)) {
		STUPID_LOG_FATAL("out of memory");
		return NULL;
	}

	stMemset(data, 0, size);

	// array header
	StMemory *mem	 = (StMemory *)data;
	mem->stride    = stride;
	mem->length    = 0;
	mem->capacity  = capacity;
	mem->type_name = type_name;

	const usize index = typeNameToIndex(mem->type_name);
	stats.total += size;
	stats.tag[index] += size;

	STUPID_DBG_SHOULD_LOG(
		if (type_name != NULL) {
			STUPID_LOG_TRACEFN("%p (%s)[%zu * %zu]", data + ST_MEMORY_HEADER_SIZE, type_name, stride, capacity);
		}
		else {
			STUPID_LOG_TRACEFN("%zu", stride * capacity);
		}
	);

	return data + ST_MEMORY_HEADER_SIZE;
}

void (stMemDealloc)(void **array STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(array);
	STUPID_NC(*array);
	StMemory *mem = ST_MEMORY_CAST(*array);
	STUPID_ASSERT(mem->stride != 0, "this shouldnt be possible");

	STUPID_LOG_TRACEFN("%p (%s)[%zu * %zu]", *array, mem->type_name, mem->stride, mem->capacity);

	usize size = mem->capacity * mem->stride + ST_MEMORY_HEADER_SIZE;
	size = (size + 31) & (-32);
	stats.total -= size;
	stats.tag[typeNameToIndex(mem->type_name)] -= size;

	stMemset(mem, 0, size);
	FREE_IMPL(mem);

	// set the array passed as an argument to NULL to avoid continued use
	*array = NULL;
}

void STUPID_ATTR_MALLOC *(stMemResize)(void **array, const usize new_capacity STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(array);
	STUPID_NC(*array);
	const StMemory *mem = ST_MEMORY_CAST(*array);
	STUPID_ASSERT(new_capacity != mem->capacity, "already at that capacity");
	STUPID_ASSERT(new_capacity != 0, "this shouldnt be possible");
	STUPID_ASSERT(mem->stride != 0, "this shouldnt be possible");

	// allocate the new array
	void *new_array = (stMemAlloc)(mem->stride, new_capacity, mem->type_name STUPID_DBG_PARAMS_NL);
	STUPID_NC(new_array);

	// subtract the header offset
	StMemory *new_mem = ST_MEMORY_CAST(new_array);
	stMemMove(new_mem, mem, mem->capacity * mem->stride + ST_MEMORY_HEADER_SIZE);

	new_mem->capacity = new_capacity;

	if (mem->type_name != NULL)
		STUPID_LOG_TRACEFN("%p (%s)[%zu->%zu]", new_array, mem->type_name, mem->capacity, new_capacity);
	else
		STUPID_LOG_TRACEFN("%p %zu->%zu", new_array, mem->stride * mem->capacity, mem->stride * new_capacity);

	stMemDeallocNL(*array);

	*array = new_array;
	return *array;
}

void (stMemAppend)(void **array, const void *data)
{
	STUPID_NC(array);
	STUPID_NC(*array);

	StMemory *mem = ST_MEMORY_CAST(*array);
	STUPID_ASSERT(mem->stride != 0, "this shouldnt be possible");

	if (mem->length >= mem->capacity) {
		// a dumb formula to decide how much to allocate
		const usize size = (16 - STUPID_CLAMP((usize)stCeil(0.5 * ((f64)mem->stride)), 1, 128));
		(stMemResize)(array, mem->capacity + size STUPID_DBG_PARAMS);
		STUPID_NC(*array);
	}

	mem = ST_MEMORY_CAST(*array);

	stMemMove(*array + mem->length * mem->stride, data, mem->stride);

	mem->length++;
}

void (stMemInsert)(void **array, const usize position, const void *data STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(array);
	STUPID_NC(*array);

	StMemory *mem = ST_MEMORY_CAST(*array);

	if (position == mem->length) {
		stMemAppend(array, data);
		return;
	}

	STUPID_ASSERT(position < mem->capacity, "index out of bounds");
	STUPID_ASSERT(position < mem->length, "index out of bounds");

	if (mem->capacity <= mem->length) {
		*array = stMemResize(*array, (mem->capacity + (u64)(16.0 / (f64)mem->stride) + 7) & (-8));
		if (*array == NULL) return;
	}

	mem = ST_MEMORY_CAST(*array);

	stMemMove(*array + position * mem->stride + mem->stride, *array + position * mem->stride, (mem->length - position) * mem->stride);
	stMemMove(*array + position * mem->stride, data, mem->stride);

	mem->length++;
}

void (stMemRemove)(void **array, const usize position, void *output STUPID_DBG_PROTO_PARAMS)
{
	STUPID_NC(array);
	STUPID_NC(*array);

	StMemory *mem = ST_MEMORY_CAST(*array);

	if (output != NULL)
		stMemMove(output, *array + position * mem->stride, mem->stride);

	STUPID_ASSERT(position < mem->capacity, "index out of bounds");
	STUPID_ASSERT(position < mem->length, "index out of bounds");

	mem = ST_MEMORY_CAST(*array);

	if (position != mem->length - 1)
	       stMemMove(*array + position * mem->stride, *array + position * mem->stride + mem->stride, (mem->length - position) * mem->stride);
	stMemset(*array + mem->length * mem->stride, 0, mem->stride);

	mem->length--;
}
