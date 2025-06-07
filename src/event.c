#include "stupid/event.h"
#include "stupid/assert.h"
#include "stupid/logger.h"
#include "stupid/memory.h"

typedef struct StEvent {
	StPFN_event pfn;
	const void *listener;
} StEvent;

static StEvent *events[ST_MAX_STUPID_EVENT_CODES] = {0};

void (stEventRegister)(const st_event_code code, const void *listener, const StPFN_event pfn STUPID_DBG_PROTO_PARAMS)
{
	STUPID_ASSERT(code < ST_MAX_STUPID_EVENT_CODES, "event code out of bounds");
	STUPID_NC(pfn);

	if (events[code] == NULL)
		events[code] = stMemAlloc(StEvent, ST_MAX_EVENTS_PER_CODE);

	StEvent e = {.pfn = pfn, .listener = listener};
	stMemAppend(events[code], e);

	STUPID_LOG_TRACEFN("registered event: code %d listener %p", code, listener);
}

void stEventUnregister(const st_event_code code, const void *listener, const StPFN_event pfn)
{
	STUPID_ASSERT(code < ST_MAX_STUPID_EVENT_CODES, "event code out of bounds");
	STUPID_NC(pfn);

	if (events[code] == NULL) return;

	for (int i = 0; i < ST_MAX_STUPID_EVENT_CODES; i++) {
		if (events[code][i].listener == listener && events[code][i].pfn == pfn) {
			stMemRemove(events[code], i, NULL);
			break;
		}
	}
}

void stEventUnregisterCode(const st_event_code code)
{
	STUPID_ASSERT(code < ST_MAX_STUPID_EVENT_CODES, "event code out of bounds");
	if (events[code] == NULL) return;
	stMemDealloc(events[code]);
}

void stEventUnregisterListener(const st_event_code code, const void *listener)
{
	if (listener == NULL) return;
	if (events[code] == NULL) return;

	for (int i = 0; i < ST_MAX_STUPID_EVENT_CODES; i++) {
		if (events[code][i].listener == listener) {
			stMemRemove(events[code], i, NULL);
			break;
		}
	}
}

void stEventDealloc(void)
{
	for (int i = 0; i < ST_MAX_STUPID_EVENT_CODES; i++) {
		if (events[i] != NULL)
			stMemDealloc(events[i]);
	}
}

void stEventFire(const st_event_code code, void *sender, const StEventData data)
{
	STUPID_ASSERT(code < ST_MAX_STUPID_EVENT_CODES, "event code out of bounds");

	if (events[code] == NULL) return;
	for (int i = 0; i < stMemLength(events[code]); i++) {
		STUPID_NC(events[code][i].pfn);
		events[code][i].pfn(code, sender, (void *)events[code][i].listener, data);
	}
}

